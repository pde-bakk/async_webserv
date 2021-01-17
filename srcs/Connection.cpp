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
#include <signal.h>
#include <cerrno>
#include <algorithm>
#include <cstring>
#include "Enums.hpp"

fd_set	readFds, // temp file descriptor list for select()
				writeFds,
				readFdsBak, // temp file descriptor list for select()
				writeFdsBak;

Connection::Connection(char *configPath) : _socketFd(), threadPool(), readmutex("read"), readbakmutex("backup read"), writemutex("write"), writebakmutex("backup write") {
#if BONUS > 0
	this->worker_amount = 0;
#endif
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&readFdsBak);
	FD_ZERO(&writeFdsBak);
	_configPath = configPath;
}

Connection::~Connection() {
	this->stopServer();
}

Connection::Connection(const Connection &obj) : _socketFd(), _configPath(), worker_amount(), threadPool(), readmutex(), readbakmutex(), writemutex(), writebakmutex() {
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
#if BONUS != 0
		std::cout << _GREEN _BOLD << this->worker_amount << " workers ";
#endif
	std::cout << "Waiting for connections..." _END << std::endl;
	while (true) {
		ConnMutex.lock();
		this->transferFD_sets();

		this->readmutex.lock();
		this->writemutex.lock();
		if (select(this->getMaxFD(), &readFds, &writeFds, 0, 0) == -1)
			throw std::runtime_error(strerror(errno));
		this->readmutex.unlock();
		this->writemutex.unlock();
		std::cout << "\tAfter Select\n";

		this->handleCLI();
		// Go through existing connections looking for data to read
		for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			Server*	s = *it;
			this->checkServer(s);
			std::vector<Client*>::iterator itc = s->_connections.begin();
			while (itc != s->_connections.end()) {
				Client* c = *itc;

				c->mut.lock();
				this->handleResponse(c);
				this->receiveRequest(c);

//				c->checkTimeout();
//				if (!c->open) {
//					readbakmutex.lock();
//					writebakmutex.lock();
//					this->_allConnections.erase(c->fd);
//					std::cout << _PURPLE "\tlets delete client at fd " << c->fd << "\n" _END;
//					s->showclients(readFds, writeFds);
//					c->mut.unlock();
//					delete *itc;
//					itc = s->_connections.erase(itc);
//					std::cout << "\tRemoved client from server->_connections\n";
//					s->showclients(readFds, writeFds);
//					ConnMutex.unlock();
//					readbakmutex.unlock();
//					writebakmutex.unlock();
//					break;
//				}
				c->mut.unlock();
				std::cout << _BLUE "\tmain loop unlocked the client mutex\n"  _END;
				++itc;
			}
		}
		this->threadPool->giveTasksToWorker(this->TaskQueue);
		ConnMutex.unlock();
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
#if BONUS != 0
	std::cout << _RED _BOLD "threadpool at " << threadPool << "\n" _END;
	this->threadPool->clear();
	std::cout << "After deleting threadPool\n";
#endif
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
#if BONUS != 0
	std::cout << _RED _BOLD "threadpool at " << threadPool << "\n" _END;
	delete this->threadPool;
	std::cout << "After deleting threadPool\n";
#endif
	std::cout << _GREEN "Server stopped gracefully.\n" << _END;
}

void Connection::loadConfiguration() {
	FD_SET(0, &readFdsBak);
	this->parsefile();
	for (std::vector<Server*>::const_iterator it = _servers.begin(); it != _servers.end(); ++it) {
//		std::cout << *(*it);
		(*it)->startListening();
		FD_SET((*it)->getSocketFd(), &readFdsBak);
		this->_allConnections.insert((*it)->getSocketFd());
	}
#if BONUS != 0
	this->threadPool = new ThreadPool(this->worker_amount, this);
#endif
}

void Connection::handleCLI() {
#if BONUS != 0
	readmutex.lock();
#endif
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
#if BONUS != 0
	readmutex.unlock();
#endif
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
#if BONUS != 0
		if (key == "workers")
			this->worker_amount = ft_atoi(value.c_str());
#endif
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

Connection::Connection() : _socketFd(), _configPath(), worker_amount(), threadPool(), readmutex(), readbakmutex(), writemutex(), writebakmutex() {

}

void Connection::transferFD_sets() {
#if BONUS != 0
	readmutex.lock();
	writemutex.lock();
	readbakmutex.lock();
	writebakmutex.lock();
#endif
	readFds = readFdsBak;
	writeFds = writeFdsBak;
#if BONUS != 0
	readmutex.unlock();
	writemutex.unlock();
	readbakmutex.unlock();
	writebakmutex.unlock();
#endif
}

void Connection::checkServer(Server* s) {
#if BONUS != 0
	readmutex.lock();
	readbakmutex.lock();
#endif
	if (FD_ISSET(s->getSocketFd(), &readFds)) {
		int clientfd = s->addConnection();
		this->_allConnections.insert(clientfd);
		FD_SET(clientfd, &readFdsBak);
		std::cout << _CYAN "i just accepted a new client\n" _END;
	}
#if BONUS != 0
	readmutex.unlock();
	readbakmutex.unlock();
#endif
}

void Connection::receiveRequest(Client *c) {
#if BONUS != 0
	readmutex.lock();
	writebakmutex.lock();
#endif
	std::cout << "checking if I want to enqueue a receiving task \n";
	if (FD_ISSET(c->fd, &readFds) && !c->DoneReading) {
		std::cout << "I do!\n";
		this->TaskQueue.push(std::make_pair("receive", c));
		std::cout << _CYAN "Enqueued receiving task on fd " << c->fd << "\n" _END;
	}
#if BONUS != 0
	readmutex.unlock();
	writebakmutex.unlock();
#endif
}

void Connection::handleResponse(Client *c) {
#if BONUS != 0
	writemutex.lock();
#endif
	if (FD_ISSET(c->fd, &writeFds)) {
		this->TaskQueue.push(std::make_pair("handle", c));
		std::cout << _CYAN "Enqueued response task on fd " << c->fd << "\n" _END;
	}
#if BONUS != 0
	writemutex.unlock();
#endif
}

bool Connection::manageClient(Client *c) {
#if BONUS != 0
	readbakmutex.lock();
	writebakmutex.lock();
#endif
	bool deletion = false;
	c->checkTimeout();
	if (!c->open) {
		time_t diff = ft::getTime() - c->lastRequest;
		std::cout << "\tDELETE: client is apparently not open, timeout diff is: " << diff << "\n" _END;
		deletion = true;
		this->_allConnections.erase(c->fd);
	}
#if BONUS != 0
	readbakmutex.unlock();
	writebakmutex.unlock();
#endif
	return deletion;
}

void Connection::deleteTimedOutClients() {
	std::vector<Server*>::iterator server_it;
	std::vector<Client*>::iterator client_it;
	for (server_it = _servers.begin(); server_it != _servers.end(); server_it++) {
		Server* s = *server_it;
		client_it = s->_connections.begin();
		while (client_it != s->_connections.end()) {
			Client* c = *client_it;
			if (this->DeleteClients.count(c->fd)) {
				_allConnections.erase(c->fd);
				client_it = s->_connections.erase(client_it);
				this->TaskSet.erase(c->fd);
				this->DeleteClients.erase(c->fd);
				std::cout << "ABOUT TO FUCKING DELETE CLIENT " << c->fd << "\n";
				delete c;
				if (s->_connections.empty())
					break;
				else
					continue;
			}
			client_it++;
		}
		for (client_it = s->_connections.begin(); client_it != s->_connections.end(); client_it++) {
		}
	}
}
