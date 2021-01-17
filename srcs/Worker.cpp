//
// Created by peerdb on 15-01-21.
//

#include "Worker.hpp"
#include <unistd.h>
#include "Connection.hpp"
#include "libftGnl.hpp"
#include <iostream>
#include "ResponseHandler.hpp"
#include "RequestParser.hpp"

//typedef void (Task)(Client*);

Worker::Worker() : id(), alive(true), conn(), TaskQueue(), Qmutex("Worker Queue"), Life("Life") {
}

Worker::~Worker() {
	this->alive = false;
	while (!TaskQueue.empty())
		TaskQueue.pop();
	Qmutex.lock();
	Life.lock();
	Qmutex.unlock();
	Life.unlock();
	std::cout << "worker #" << id << " dies\n";
}

void Worker::IOruntime(int index, Connection* x) {
	this->conn = x;
	this->id = index;
	Client*		c;
	std::pair<std::string, Client*> task;
	Life.lock();
	while (this->alive) {
		Life.unlock();
		this->Qmutex.lock();
		if (this->TaskQueue.empty()) {
			this->Qmutex.unlock();
			this->Life.lock();
			usleep(1000);
			continue;
		}
		task = this->TaskQueue.front();
		this->TaskQueue.pop();
		this->Qmutex.unlock();
		c = task.second;
		c->mut.lock();
		std::cout << _YELLOW "Worker LOCKED client mutex\n" _END;
		if (task.first == "receive")
			handleClientRequest(c);
		else if (task.first == "handle") {
			handleClientResponse(c); }

		c->mut.unlock();
		std::cout << _YELLOW "Worker just unlocked client mutex\n" _END;
		Life.lock();
	}
	Life.unlock();
	std::cout <<_PURPLE "IOruntime is over for thread #" << id << "\n" _END;
}

void	Worker::handleClientRequest(Client* c) {
	std::cout << _WHITE  _BOLD "gonna read on fd " << c->fd << "\n";
	std::cout << "Read is set: " << FD_ISSET(c->fd, &readFds) << "\n";
	if (c->receiveRequest() == 1 && ft::checkIfEnded(c->req)) {
		std::cout << c->fd << " done reading on " << c->fd << "\n";
		conn->writebakmutex.lock();
		FD_SET(c->fd, &writeFdsBak);
		conn->writebakmutex.unlock();
	}

	conn->ConnMutex.lock();
	std::cout << "taskset has " << conn->TaskSet.size() << " tasks\n";
	conn->TaskSet.erase(c->fd);
	std::cout << "And now it only has " << conn->TaskSet.size() << " tasks\n";
	conn->ConnMutex.unlock();
	std::cout << "Read is set: " << FD_ISSET(c->fd, &readFds) << "\n";

//	std::cout << _WHITE _BOLD << "fd " << c->fd << "'s request is: \n" << c->req << std::endl;
std::cout << _END;
}

void	Worker::handleClientResponse(Client* c) {
	std::cout << _BOLD _PURPLE "HANDLING CLIENT RESPONSE\n" _END;
	RequestParser					requestParser;
	ResponseHandler					responseHandler;
	std::string						response;

	c->parsedRequest = requestParser.parseRequest(c->req);
	c->parsedRequest.server = c->parent;
	c->parsedRequest.location = c->parent->matchlocation(c->parsedRequest.uri);

	try {
		response = responseHandler.handleRequest(c->parsedRequest);
		c->sendReply(response.c_str(), c->parsedRequest);
		std::cout << _RED << response << _END "\n";
		response.clear();
		c->reset();
	} catch (std::exception& e) {
		std::cout << _RED _BOLD "setting client " << c->fd << " to connection closed because responsehandler threw exception\n" _END;
		c->open = false;
	}
	conn->writebakmutex.lock();
	FD_CLR(c->fd, &writeFdsBak);
	conn->writebakmutex.unlock();

	conn->ConnMutex.lock();
	conn->TaskSet.erase(c->fd);
	conn->DeleteClients.insert(c->fd);
	conn->ConnMutex.unlock();

}

void Worker::giveTask(std::pair<std::string, Client *>& NewTask) {
	this->Qmutex.lock();
	this->TaskQueue.push(NewTask);
	this->Qmutex.unlock();
}
