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

Worker::Worker() : id(), alive(true), conn(), WorkerTaskQueue(), Qmutex(this->WorkerTaskQueue), Life(this->alive) {
}

Worker::Worker(int id, Connection *connection) : id(id), alive(true), conn(connection), WorkerTaskQueue(), Qmutex(this->WorkerTaskQueue), Life(this->alive) {

}

Worker::~Worker() {
	this->alive = false;
	while (!WorkerTaskQueue.empty())
		WorkerTaskQueue.pop();
	{
		Mutex::Guard<TaskQueue>	QMutexGuard(this->Qmutex);
		Mutex::Guard<bool>		LifeGuard(this->Life);
	}
	std::cout << "worker #" << id << " dies\n";
}

void Worker::IOruntime() {
	Client*	c;
	Task	task;
	while (true) {
		usleep(100000);
		{
			Mutex::Guard<bool> LifeGuard(this->Life);
			if (!this->alive) {
				break;
			}
		}
		{
			Mutex::Guard<TaskQueue>	QMutexGuard(Qmutex);
			if (this->WorkerTaskQueue.empty()) {
				usleep(300000);
				continue;
			}
			task = this->WorkerTaskQueue.front();
			this->WorkerTaskQueue.pop();
		}
		c = task.second;
		int clientfd;
		bool DeletionNecessary = false;
		{
			Mutex::Guard<Client> ClientGuard(c->mut);
			clientfd = c->fd;
			if (task.first == "receive")
				DeletionNecessary = handleClientRequest(c);
			else if (task.first == "handle")
				DeletionNecessary = handleClientResponse(c);
		}
		if (clientfd > 2)
			this->CommunicateWithConnection(clientfd, DeletionNecessary);
	}
}

bool	Worker::handleClientRequest(Client* c) {
	if (c->receiveRequest() == 1 && ft::checkIfEnded(c->req)) {
		Mutex::Guard<fd_set>	WriteBakGuard(conn->writebakmutex);
		FD_SET(c->fd, &writeFdsBak);
	}
	return (!c->open);
}

bool	Worker::handleClientResponse(Client* c) {
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
		response.clear();
		c->reset();
	} catch (std::exception& e) {
		std::cout << _RED _BOLD "setting client " << c->fd << " to connection closed because responsehandler threw exception\n" _END;
		c->open = false;
	}
	Mutex::Guard<fd_set>	WriteBakGuard(conn->writebakmutex);
	FD_CLR(c->fd, &writeFdsBak);
	return true;
}

void Worker::giveTask(const Task& NewTask) {
	Mutex::Guard<TaskQueue>	QMutexGuard(this->Qmutex);
	this->WorkerTaskQueue.push(NewTask);
}

void Worker::CommunicateWithConnection(int clientfd, bool DeletionNecessary) {
	Mutex::Guard<std::set<int> >	HandleGuard(conn->cHandleMut);
	this->conn->ClientsBeingHandled.erase(clientfd);

	if (DeletionNecessary) {
		Mutex::Guard<std::set<int> >	DelGuard(conn->cDelMut);
		this->conn->ClientsToBeDeleted.insert(clientfd);
	}
}
