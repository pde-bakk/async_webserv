//
// Created by peerdb on 13-01-21.
//

#include "Client.hpp"
#include "Connection.hpp"
#include "Server.hpp"
#include "libftGnl.hpp"
#include <cerrno>
#include <cstring>

int g_sigpipe;

Client::Client(Server* S) : parent(S), fd(), port(), open(true), addr(), size(sizeof(addr)), lastRequest(0), parsedRequest(), mut("client"), TaskInProgress(false), DoneReading() {
	bzero(&addr, size);
	this->fd = accept(S->getSocketFd(), (struct sockaddr*)&addr, &size);
	if (this->fd == -1) {
		std::cerr << _RED _BOLD "Error accepting connection\n" _END;
		throw std::runtime_error(strerror(errno));
	}
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		std::cerr << _RED _BOLD "Error setting connection fd to be nonblocking\n" _END;
		throw std::runtime_error(strerror(errno));
	}
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		std::cerr << _RED _BOLD "Error setting connection fd socket options\n" _END;
		throw std::runtime_error(strerror(errno));
	}
	this->host = inet_ntoa(addr.sin_addr);
	this->port = htons(addr.sin_port);
	this->ipaddress = host + ':' + ft::inttostring(port);

	FD_SET(this->fd, &readFdsBak);

//	if (CONNECTION_LOGS)
		std::cerr << _YELLOW "Opened a new client for " << fd << " at " << ipaddress << std::endl << _END;
}

Client::~Client() {
	std::cout << "deleting client on fd " << fd << "\n";
	close(fd);
	mut.lock();
	mut.unlock();
	fd = -1;
	req.clear();
	this->parsedRequest.clear();
	FD_CLR(this->fd, &readFdsBak);
	FD_CLR(this->fd, &writeFdsBak);
}

int Client::receiveRequest() {
	char buf[BUFLEN + 1];
	int recvRet;
	bool recvCheck(false);

	// Loop to receive complete request, even if buffer is smaller
	ft_memset(buf, 0, BUFLEN);
	this->resetTimeout();
	while ((recvRet = recv(this->fd, buf, BUFLEN, 0)) > 0) {
		buf[recvRet] = '\0';
		this->req.append(buf);
		recvCheck = true;
		usleep(1000);
	}
	if (recvRet == -1) {
		std::cout << _RED "After recv loop, recvRet is " << recvRet << ", and recvCheck is " << std::boolalpha << recvCheck << "\n" _END;
		std::cout << strerror(errno) << "\n";
	}
	if (recvRet == 0) { // socket closed
		std::cerr << _RED _BOLD "Socket closed\n" _END;
		this->open = false;
		return (0);
	}
	else if (!recvCheck) {
		if (!DoneReading)
			DoneReading = true;
		else
			std::cout << _RED "Closing connection, recvCheck is " << recvCheck << ", received " << recvRet << " bytes\n" _END;
		return (0);
	}
	return (1);
}

void Client::sendReply(const char* msg, request_s& request) const {
	long	bytesToSend = ft_strlen(msg),
			bytesSent(0),
			sendRet;

	g_sigpipe = 0;
	while (bytesToSend > 0) {
		sendRet = send(this->fd, msg + bytesSent, bytesToSend, 0);
		if (sendRet == -1) {
			if (g_sigpipe == 0 && bytesToSend != 0)
				continue;
			throw std::runtime_error(strerror(errno));
		}
		bytesSent += sendRet;
		bytesToSend -= sendRet;
	}
	static int i = 1, post = 1;
	std::cerr << _PURPLE "sent response for request #" << i++ << " (" << methodAsString(request.method);
	if (request.method == POST)
		std::cerr << " #" << post++;
	std::cerr << ").\n" _END;
}

void Client::resetTimeout() {
	this->lastRequest = ft::getTime();
}

void Client::checkTimeout() {
	if (this->lastRequest) {
		time_t diff = this->getTimeDiff();
		std::cout << "current diff is " << diff << "\n";
		std::cout << "open is " << open << "\n";
		if (diff > 100000) {
			this->open = false;
			std::cout <<  _PURPLE _BOLD "TIMEOUT TOO BIG, SETTING CONECTION TO FALSE\n" _END;
		}
	}
}

void Client::reset() {
	if (this->parsedRequest.headers[CONNECTION] == "close") {
		std::cout << "Setting connection to closed because of request header\n";
		if (CONNECTION_LOGS)
			std::cerr << "We ain't resetting, we're closing this client, baby" << std::endl;
		this->open = false;
		return;
	} else if (!this->open)
		return;
	if (CONNECTION_LOGS)
		std::cerr << "Resetting client!\n";
	this->open = true;
	req.clear();
	lastRequest = 0;
	this->parsedRequest.clear();
}

Client::Client() : parent(), fd(), port(), open(), addr(), size(), lastRequest(), TaskInProgress(), DoneReading() {

}

void Client::breakOnSIGPIPE(int) {
	std::cerr << _RED _BOLD << "sending response failed. shutting down connection.\n" _END;
	g_sigpipe = 1;
}

time_t Client::getTimeDiff() const {
	std::cout << "Time Diff is " << ft::getTime() - this->lastRequest << "\n";
	return ft::getTime() - this->lastRequest;
}

Client &Client::operator=(const Client &x) {
	if (this != &x) {
		parent = x.parent;
		fd = x.fd;
		port = x.port;
		open = x.open;
		addr = x.addr;
		size = x.size;
		req = x.req;
		host = x.host;
		ipaddress = x.ipaddress;
		lastRequest = x.lastRequest;
		parsedRequest = x.parsedRequest;
		mut = x.mut;
		TaskInProgress = x.TaskInProgress;
		DoneReading = x.DoneReading;
	}
	return *this;
}

Client::Client(const Client &x) : parent(), fd(), port(), open(), addr(), size(), lastRequest(), TaskInProgress(), DoneReading() {
	*this = x;
}

