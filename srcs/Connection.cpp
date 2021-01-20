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

Connection::Connection(char* configPath) : _socketFd(-1), _configPath(), alive(true)
#if BONUS
		,worker_amount(0), threadPool(NULL),
		readmutex(readFds), readbakmutex(readFdsBak), writemutex(writeFds), writebakmutex(writeFdsBak),
		cHandleMut(ClientsBeingHandled), AliveMut(this->alive)
#endif
{
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&readFdsBak);
	FD_ZERO(&writeFdsBak);
	_configPath = configPath;
}

Connection::~Connection() {
	this->stopServer();
}


Connection::Connection() : _socketFd(-1), _configPath(), alive(true)
#if BONUS
		,worker_amount(0), threadPool(NULL),
		readmutex(readFds), readbakmutex(readFdsBak), writemutex(writeFds), writebakmutex(writeFdsBak),
						   cHandleMut(ClientsBeingHandled), AliveMut(this->alive)
#endif
{ }

Connection::Connection(const Connection& obj) : _socketFd(), _configPath(), alive(true)
#if BONUS
		,worker_amount(0), threadPool(NULL),
		readmutex(readFds), readbakmutex(readFdsBak), writemutex(writeFds), writebakmutex(writeFdsBak),
												cHandleMut(ClientsBeingHandled), AliveMut(this->alive)
#endif
{
	*this = obj;
}

Connection& Connection::operator=(const Connection &obj) {
	if (this != &obj) {
		this->_socketFd = obj._socketFd;
		this->_servers = obj._servers;
		this->_configPath = obj._configPath;
		this->_allConnections = obj._allConnections;
	}
	return *this;
}

// ------------------ Process Handling ------------------------
Connection*	THIS;
void Connection::startServer() {
	loadConfiguration();
	THIS = this;
	signal(SIGINT, Connection::signalServer);
	signal(SIGPIPE, Client::breakOnSIGPIPE);
#if BONUS
	if (worker_amount > 0)
		this->multiThreadedListening();
	else
		this->startListening();
#else
	this->startListening();
#endif
}

void Connection::signalServer(int) {
	std::cerr << _RED _BOLD "\nSignaled to stop the server.\n" _END;
	Mutex::Guard<bool>	AliveGuard(THIS->AliveMut);
	THIS->alive = false;
}

void Connection::stopServer() {
	// Go through existing connections and close them
	std::vector<Server*>::iterator	sit;
	for (sit = _servers.begin(); sit != _servers.end(); ++sit) {
		(*sit)->clearclients();
		delete *sit;
		*sit = NULL;
	}
	_servers.clear();
	FD_ZERO(&readFds);
	FD_ZERO(&writeFds);
	FD_ZERO(&readFdsBak);
	FD_ZERO(&writeFdsBak);
#if BONUS
	delete this->threadPool;
	this->threadPool = NULL;
#endif
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
#if BONUS
	this->threadPool = new ThreadPool(this->worker_amount, this);
#endif
}

void Connection::handleCLI() {
#if BONUS
	Mutex::Guard<fd_set>	ReadGuard(this->readmutex);
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

#if BONUS
		if (key == "workers")
			this->worker_amount = ft_atoi(value.c_str());
#endif

		if (key == "server" && value == "{") {
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

void Connection::startListening() {
	std::cout << "Waiting for connections..." << std::endl;
	while (alive) {
		if (!alive)
			break;
		readFds = readFdsBak;
		writeFds = writeFdsBak;
//		this->_servers.front()->showclients(_readFds, _writeFds);
		if (::select(this->getMaxFD(), &readFds, &writeFds, 0, 0) == -1)
			throw std::runtime_error(strerror(errno));
		if (FD_ISSET(0, &readFdsBak))
			this->handleCLI();
		// Go through existing connections looking for data to read
		for (std::vector<Server*>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			Server*	s = *it;
			if (FD_ISSET(s->getSocketFd(), &readFds)) {
				int clientfd = s->addConnection(this);
				this->_allConnections.insert(clientfd);
				FD_SET(clientfd, &readFdsBak);
			}
			std::vector<Client*>::iterator itc = s->_connections.begin();
			while (itc != s->_connections.end()) {
				Client* c = *itc;
				if (FD_ISSET(c->fd, &readFds)) {
					if (c->receiveRequest() == 1 && ft::checkIfEnded(c->req)) {
						FD_SET(c->fd, &writeFdsBak);
					}
				}
				if (FD_ISSET(c->fd, &writeFds)) {
					RequestParser					requestParser;
					ResponseHandler					responseHandler;
					std::string						response;

					c->parsedRequest = requestParser.parseRequest(c->req);
					c->parsedRequest.server = c->parent;
					c->parsedRequest.location = c->parent->matchlocation(c->parsedRequest.uri);

					try {
						response = responseHandler.handleRequest(c->parsedRequest);
						c->sendReply(response.c_str(), c->parsedRequest);
						response.clear();
						c->reset();
					} catch (std::exception& e) {
						c->open = false;
					}
					FD_CLR(c->fd, &writeFdsBak);
				}
				c->checkTimeout();
				if (!c->open) {
					FD_CLR(c->fd, &readFdsBak);
					FD_CLR(c->fd, &writeFdsBak);
					this->_allConnections.erase(c->fd);
					delete *itc;
					s->_connections.erase(itc);
					if (s->_connections.empty())
						break;
					else {
						itc = s->_connections.begin();
						continue;
					}
				}
				++itc;
			}
		}
	}
}

#if BONUS
void Connection::multiThreadedListening() {
	std::cout << "Waiting for connections... with " << this->worker_amount << " workers" _END << std::endl;
	while (alive) {
		{
			Mutex::Guard<bool>	AliveGuard(this->AliveMut);
			if (!alive)
				break;
		}
		this->transferFD_sets();
		this->select();
		this->handleCLI();
		// Go through existing connections looking for data to read
		for (std::vector<Server *>::iterator it = _servers.begin(); it != _servers.end(); ++it) {
			Server *s = *it;
			this->checkServer(s);
			std::vector<Client *>::iterator itc = s->_connections.begin();
			while (itc != s->_connections.end()) {
				Client *c = *itc;
				{
					Mutex::Guard<std::map<int, Status> > HandleGuard(this->cHandleMut);
					if (this->ClientsBeingHandled.count(c->fd) == 1) {
						++itc;
						continue;
					}
				}
				{
					std::cout << "before handleresponse\n";
					this->handleResponse(c);
					std::cout << "before receiveRequest\n";
					this->receiveRequest(c);
					std::cout << "after receiveRequest\n";
				}
				++itc;
			}
		}
		this->deleteTimedOutClients();
	}
}

void Connection::transferFD_sets() {
	Mutex::Guard<fd_set>	readguard(this->readmutex),
							writeguard(this->writemutex),
							readbakguard(this->readbakmutex),
							writebakguard(this->writebakmutex);
	readFds = readFdsBak;
	writeFds = writeFdsBak;
}

void Connection::select() {
	std::string selecterror("Select failed because of ");
	struct timeval tv = { 0, 500};
	int ret;
	Mutex::Guard<fd_set>	readguard(this->readmutex),
							writeguard(this->writemutex);
	if ((ret = ::select(this->getMaxFD(), &readFds, &writeFds, 0, &tv)) == -1)
		throw std::runtime_error(selecterror + strerror(errno));
//	std::cout << "select returned " << ret << "\n";
	(void)ret;
}

void Connection::checkServer(Server* s) {
	Mutex::Guard<fd_set>	readguard(this->readmutex);
	if (FD_ISSET(s->getSocketFd(), &readFds)) {
		int clientfd = s->addConnection(this);
		this->_allConnections.insert(clientfd);
	}
}

void Connection::receiveRequest(Client *c) {
	Mutex::Guard<fd_set>	readguard(this->readmutex),
							writebakguard(this->writebakmutex);
	if (FD_ISSET(c->fd, &readFds)) {
		std::cout << "readfd is set\n";
		this->threadPool->AddTaskToQueue(new std::pair<std::string, Client*>("receive", c));
	}
}

void Connection::handleResponse(Client *c) {
	Mutex::Guard<fd_set>	writeguard(this->writemutex);
	if (FD_ISSET(c->fd, &writeFds)) {
		std::cout << "writefd is set\n";
		this->threadPool->AddTaskToQueue(new std::pair<std::string, Client*>("handle", c));
	}
}

void Connection::deleteTimedOutClients() {
	std::vector<Server*>::iterator server_it;
	std::vector<Client*>::iterator client_it;
	for (server_it = this->_servers.begin(); server_it != _servers.end(); server_it++) {
		Server* serv = *server_it;
		for (client_it = serv->_connections.begin(); client_it != serv->_connections.end(); client_it++) {
			Client* cli = *client_it;
			bool isOpen = true,
				 isBeingHandled = false,
				 ClearWrite = false,
				 SetWrite = false;
			{
				Mutex::Guard<std::map<int, Status> >	ClientsHandleGuard(this->cHandleMut);
				if (this->ClientsBeingHandled.count(cli->fd) == 1) {
					if (this->ClientsBeingHandled[cli->fd] == DONE_WRITING) {
						ClearWrite = true;
						this->ClientsBeingHandled.erase(cli->fd);
					} else if (this->ClientsBeingHandled[cli->fd] == DONE_READING) {
						SetWrite = true;
						this->ClientsBeingHandled.erase(cli->fd);
					} else
						isBeingHandled = true;
				}
			}
			{
				if (ClearWrite) {
					std::cout << "Clearing writebak\n";
					Mutex::Guard<fd_set> WriteBackupGuard(writebakmutex);
					FD_CLR(cli->fd, &writeFdsBak);
				}
				if (SetWrite) {
					std::cout << "Setting writebak\n";
					Mutex::Guard<fd_set> WriteBackupGuard(writebakmutex);
					FD_SET(cli->fd, &writeFdsBak);
				}
			}
			{
				Mutex::Guard<Client> ClientGuard(cli->mut);
				if (!cli->open)
					isOpen = false;
			}
			{
				if (!isOpen && !isBeingHandled) {
					serv->deleteclient(client_it);
					if (serv->_connections.empty())
						break;
				}
			}
		}
	}
}

#endif
