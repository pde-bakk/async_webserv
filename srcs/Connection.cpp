/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.cpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: sam <sam@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/03 15:26:44 by sam           #+#    #+#                 */
/*   Updated: 2021/01/14 16:34:22 by tuperera      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"
#include <iostream>
#include <sys/stat.h>
#include <zconf.h>
#include <Colours.hpp>
#include "ResponseHandler.hpp"
#include "libftGnl.hpp"
#include <csignal>
#include <cerrno>
#include <algorithm>
#include <cstring>
#include "Enums.hpp"

fd_set	readFds, // temp file descriptor list for select()
				writeFds,
				readFdsBak, // temp file descriptor list for select()
				writeFdsBak;

Connection::Connection(char* configPath) : _socketFd(), _configPath(), worker_amount(0), threadPool(),
										   readmutex(), readbakmutex(), writemutex(), writebakmutex(),
										   cDelMut(), cHandleMut() {
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&readFdsBak);
	FD_ZERO(&writeFdsBak);
	_configPath = configPath;
}

Connection::~Connection() {
	this->stopServer();
}

Connection::Connection(const Connection& obj) : _socketFd(), _configPath(), worker_amount(), threadPool(),
												readmutex(), readbakmutex(), writemutex(), writebakmutex(),
												cDelMut(), cHandleMut() {
	*this = obj;
}

Connection& Connection::operator= (const Connection &obj) {
	if (this != &obj) {
		this->_socketFd = obj._socketFd;
		this->_servers = obj._servers;
		this->_configPath = obj._configPath;
		this->_allConnections = obj._allConnections;
	}
	return *this;
}

void Connection::startListening() {
	std::cout << _GREEN _BOLD << this->worker_amount << " workers ";
	std::cout << "Waiting for connections..." _END << std::endl;
	while (true) {
		this->transferFD_sets();
		std::cout << "before select call\n";
		this->select();
		std::cout << "after select\n";
		this->handleCLI();
		// Go through existing connections looking for data to read
		for (std::vector<Server *>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			Server *s = *it;
			this->checkServer(s);
			std::vector<Client *>::iterator itc = s->_connections.begin();
			while (itc != s->_connections.end()) {
				Client *c = *itc;
				{
					Mutex::Guard	HandleGuard(this->cHandleMut);
					Mutex::Guard	DeletionGuard(this->cDelMut);
					if (this->ClientsBeingHandled.count(c->fd) == 1 || this->ClientsToBeDeleted.count(c->fd) == 1) {
//						std::cout << "Not handling " << c->fd << " this loop because " << (this->ClientsToBeDeleted.count(c->fd) ? "it should be deleted" : "its already being handled") << "\n";
						++itc;
						continue;
					}
				}
				{
					this->handleResponse(c);
					this->receiveRequest(c);
				}
				++itc;
				std::cout << _BLUE "\tmain loop unlocked the client mutex\n"  _END;
			}
		}
		this->threadPool->giveTasksToWorker();
		this->deleteTimedOutClients();
	}
}

// ------------------ Process Handling ------------------------
Connection*	THIS;
void Connection::startServer() {
	loadConfiguration();
	THIS = this;
	signal(SIGINT, Connection::signalServer);
	signal(SIGPIPE, Client::breakOnSIGPIPE);
	startListening();
}

void Connection::signalServer(int n) {
	std::cerr << _RED _BOLD "\nSignaled to stop the server.\n" _END;
	THIS->stopServer();
	exit(n);
}

void Connection::stopServer() {
	// Go through existing connections and close them
	std::cout << _RED _BOLD "threadpool at " << threadPool << "\n" _END;
	this->threadPool->joinThreads();

	std::vector<Server*>::iterator	sit;
	for (sit = _servers.begin(); sit != _servers.end(); ++sit) {
		(*sit)->clearclients();
		delete *sit;
	}
	_servers.clear();
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&readFdsBak);
	FD_ZERO(&writeFdsBak);
	std::cout << _RED _BOLD "threadpool at " << threadPool << "\n" _END;
	delete this->threadPool;
	std::cout << "After deleting threadPool\n";
	std::cout << _GREEN "Server stopped gracefully.\n" << _END;
}

void Connection::loadConfiguration() {
	FD_SET(0, &readFdsBak);
	this->parsefile();
	for (std::vector<Server*>::const_iterator it = _servers.begin(); it != _servers.end(); ++it) {
		std::cout << *(*it);
		(*it)->startListening();
		FD_SET((*it)->getSocketFd(), &readFdsBak);
		this->_allConnections.insert((*it)->getSocketFd());
	}
	this->threadPool = new ThreadPool(this->worker_amount, this);
}

void Connection::handleCLI() {
	readmutex.lock();

	if (FD_ISSET(0, &readFds)) {
		std::string input;
		std::getline(std::cin, input);

		if (input == "exit") {
			std::cout << "Shutting down..." << std::endl;
			stopServer();
			exit(0);
		}
		else if (input == "restart") {
			std::cout << "Restarting server..." << std::endl;
			stopServer();
			startServer();
		}
		else if (input == "help") {
			std::cout << "Please use one of these commands:\n"
						 "\n"
						 "   help              For this help\n"
						 "   exit              Shut down the server\n"
						 "   restart           Restart the server\n" << std::endl;
		}
		else {
			std::cout << "Command \"" << input << "\" not found. Type \"help\" for available commands" << std::endl;
		}
	}
	readmutex.unlock();
}

int Connection::getMaxFD() {
	return (*std::max_element(this->_allConnections.begin(), this->_allConnections.end()) + 1);
}

void Connection::parsefile() {
	std::string		str;
	struct stat statstruct = {};
	int fd;
	if (this->_configPath && stat(_configPath, &statstruct) != -1)
		fd = open(_configPath, O_RDONLY);
	else
		fd = open("configfiles/42test.conf", O_RDONLY);
	if (fd < 0)
		return;

	while (ft::get_next_line(fd, str) > 0) {
		std::string key, value;
		if (ft::is_first_char(str) || str.empty())
			continue ;
		ft::get_key_value(str, key, value);

		if (key == "workers")
			this->worker_amount = ft_atoi(value.c_str());

		else if (key == "server" && value == "{") {
			try {
				Server *tmp = new Server();
				if (!str.empty() &&
					str.compare(str.find_first_not_of(" \t\n"), ft_strlen("server {"), "server {") == 0) {
					tmp->setup(fd);
					this->_servers.push_back(tmp);
				}
			}
			catch (std::exception &e) {
				std::cerr << "Exception in parse try block: " << e.what() << std::endl;
				close(fd);
				exit(1);
			}
		}
	}
	if (close(fd) == -1)
		throw std::runtime_error("closing config file failed");
}

Connection::Connection() : _socketFd(), _configPath(), worker_amount(), threadPool(),
						   readmutex(), readbakmutex(), writemutex(), writebakmutex(),
						   cDelMut(), cHandleMut() {

}

void Connection::transferFD_sets() {
#if BONUS != 0
	Mutex::Guard	readguard(this->readmutex),
					writeguard(this->writemutex),
					readbakguard(this->readbakmutex),
					writebakguard(this->writebakmutex);
#endif
	readFds = readFdsBak;
	writeFds = writeFdsBak;
}

void Connection::select() {
	std::string selecterror("Select failed because of ");
	struct timeval tv = { 1, 5000};
#if BONUS != 0
	Mutex::Guard	readguard(this->readmutex),
					writeguard(this->writemutex);
#endif
	std::cout << "now entering select call, i have the guards\n";
	if (::select(this->getMaxFD(), &readFds, &writeFds, 0, &tv) == -1)
		throw std::runtime_error(selecterror + strerror(errno));
}

void Connection::checkServer(Server* s) {
#if BONUS != 0
	Mutex::Guard	readguard(this->readmutex);
#endif
	if (FD_ISSET(s->getSocketFd(), &readFds)) {
		std::cout << "lets accept a new client!\n";
		int clientfd = s->addConnection(this);
		this->_allConnections.insert(clientfd);
//		FD_SET(clientfd, &readFdsBak); I'm handling this in the Client constructor already
		std::cout << _CYAN "i just accepted a new client\n" _END;
	}
}

void Connection::receiveRequest(Client *c) {
#if BONUS != 0
	Mutex::Guard	readguard(this->readmutex),
					writebakguard(this->writebakmutex);
#endif
	std::cout << "\tchecking if I want to enqueue a receiving task \n";
	if (FD_ISSET(c->fd, &readFds) && !c->DoneReading) {
		std::cout << "\tI do!\n";
		this->threadPool->AddTaskToQueue(std::make_pair("receive", c));
		std::cout << _CYAN "\tEnqueued receiving task on fd " << c->fd << "\n" _END;
	}
}

void Connection::handleResponse(Client *c) {
#if BONUS != 0
	Mutex::Guard	writeguard(this->writemutex);
#endif
	if (FD_ISSET(c->fd, &writeFds)) {
		this->threadPool->AddTaskToQueue(std::make_pair("handle", c));
		std::cout << _CYAN "\tEnqueued response task on fd " << c->fd << "\n" _END;
	}
}

void Connection::deleteTimedOutClients() {
	std::vector<Server*>::iterator server_it;
	std::vector<Client*>::iterator client_it;
	for (server_it = this->_servers.begin(); server_it != _servers.end(); server_it++) {
		Server* serv = *server_it;
		for (client_it = serv->_connections.begin(); client_it != serv->_connections.end(); client_it++) {
			Client* cli = *client_it;
//			Mutex::Guard ClientGuard(cli->mut);
			Mutex::Guard	ClientsDelGuard(this->cDelMut);
			if (this->ClientsToBeDeleted.count(cli->fd) == 1) {
				std::cout << "\tLet's delete client with fd " << cli->fd << "\n";
				this->_allConnections.erase(cli->fd);
				this->ClientsToBeDeleted.erase(cli->fd);
				{
					Mutex::Guard ClientsHandleGuard(this->cHandleMut);
					this->ClientsBeingHandled.erase(cli->fd);
				}
				serv->deleteclient(client_it);
				std::cout << "\tAfter serv->deleteclient(client_it)\n";
				if (serv->_connections.empty())
					break;
			}
		}
	}
}
