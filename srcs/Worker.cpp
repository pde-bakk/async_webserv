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

Worker::Worker() : id(), alive(true), conn(), TaskQ(NULL), Life(this->alive) {
}

Worker::Worker(int id, Connection *connection, ThreadSafeQueue<Task*>* Q) : id(id), alive(true), conn(connection), TaskQ(Q), Life(this->alive) {
}

Worker::~Worker() {
	this->alive = false;
	std::cout << "worker #" << id << " dies\n";
}

void Worker::IOruntime() {
	Client*	c;
	Task*	task;
	while (true) {
		task = NULL;
		{
			Mutex::Guard<bool> LifeGuard(this->Life);
			if (!this->alive) {
				break;
			}
		}
		{
			task = TaskQ->pop();
			if (task == NULL) {
				usleep(10000);
				continue;
			}
		}
		c = task->second;
		int clientfd = 0;
		Status	status = IN_PROGRESS;
		{
			Mutex::Guard<Client> ClientGuard(c->mut);
			clientfd = c->fd;
			if (task->first == "receive")
				status = handleClientRequest(c);
			else if (task->first == "handle")
				status = handleClientResponse(c);
		}
		{
			Mutex::Guard<fd_set> WriteBakGuard(conn->writebakmutex);
			if (status == DONE_READING) {
				FD_SET(clientfd, &writeFdsBak);
				std::cout << _YELLOW "clientfd has been set to writeable now\n";
			}
			if (status == DONE_WRITING) {
				FD_CLR(clientfd, &writeFdsBak);
				std::cout << _RED "Removing " << clientfd << " from writefdsbak\n";
			}
		}
		std::cout << "Client status is " << StatusToString(status) << "\n";
		if (clientfd > 2)
			this->CommunicateWithConnection(clientfd, status);
		delete task;
	}
}

Status	Worker::handleClientRequest(Client* c) {
	if (c->receiveRequest() == 1 && ft::checkIfEnded(c->req)) {
//		std::cout << "Yes we done reading bby\n";
//		std::cout << _WHITE _BOLD << c->req << "\n" _END;
		std::cout << "Done reading\n";
		return DONE_READING;
	}
	std::cout << "Still not done reading bby\n";
	std::cout << _WHITE << "size is " << c->req.size() << "\n" _END;
	return IN_PROGRESS;
}

Status	Worker::handleClientResponse(Client* c) {
	RequestParser					requestParser;
	ResponseHandler					responseHandler;
	std::string						response;

	c->parsedRequest = requestParser.parseRequest(c->req);
	c->parsedRequest.server = c->parent;
	c->parsedRequest.location = c->parent->matchlocation(c->parsedRequest.uri);

	try {
		response = responseHandler.handleRequest(c->parsedRequest);
		c->sendReply(response.c_str(), c->parsedRequest);
		c->reset();
	} catch (std::exception& e) {
		std::cout << _RED _BOLD "setting client " << c->fd << " to connection closed because responsehandler threw exception\n" _END;
		std::cout << _RED << response << "\n" _END;
		c->open = false;
	}
	response.clear();
	return DONE_WRITING;
}

void Worker::CommunicateWithConnection(int clientfd, Status status) {
	{
		Mutex::Guard<std::map<int, Status> >	HandleGuard(conn->cHandleMut);
		this->conn->ClientsBeingHandled[clientfd] = status; // its done being handled
	}
//	if (!open) {
//		Mutex::Guard<std::map<int, bool> >	DelGuard(conn->cDelMut);
//		this->conn->ClientsToBeDeleted[clientfd] = true; // this value doesnt matter
//	}
}
