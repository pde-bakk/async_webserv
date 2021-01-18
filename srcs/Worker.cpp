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

Worker::Worker() : id(), alive(true), conn(), WorkerTaskQueue(), Qmutex("Qmutex"), Life("Life") {
}

Worker::Worker(int id, Connection *connection) : id(id), alive(true), conn(connection), WorkerTaskQueue(), Qmutex("Qmutex"), Life("WorkerLife") {

}

Worker::~Worker() {
	this->alive = false;
	while (!WorkerTaskQueue.empty())
		WorkerTaskQueue.pop();
	{
		Mutex::Guard	QMutexGuard(Qmutex);
		Mutex::Guard	LifeGuard(Life);
	}
	std::cout << "worker #" << id << " dies\n";
}

void Worker::IOruntime() {
	Client*		c;
	std::pair<std::string, Client*> task;
	while (true) {
		usleep(100000);
		{
			Mutex::Guard LifeGuard(this->Life, "Workerlife");
			if (!this->alive) {
				break;
			}
		}
		{
			Mutex::Guard	QMutexGuard(Qmutex, "Worker Qmutex");
			if (this->WorkerTaskQueue.empty()) {
				usleep(100000);
				continue;
			}
			std::cout << "Lets call queue.front()\n";
			task = this->WorkerTaskQueue.front();
			this->WorkerTaskQueue.pop();
			std::cout << "Close worker q guard pls?\n";
		}
		c = task.second;
		int clientfd;
		bool DeletionNecessary;
		{
			Mutex::Guard ClientGuard(c->mut);
			std::cout << _YELLOW "Worker LOCKED client mutex\n" _END;
			clientfd = c->fd;
			if (task.first == "receive")
				DeletionNecessary = handleClientRequest(c);
			else if (task.first == "handle")
				DeletionNecessary = handleClientResponse(c);
			std::cout << _YELLOW "Worker just unlocked client mutex\n" _END;
		}
		if (clientfd > 2)
			this->CommunicateWithConnection(clientfd, DeletionNecessary);
	}
	std::cout <<_PURPLE "IOruntime is over for thread #" << id << "\n" _END;
}

bool	Worker::handleClientRequest(Client* c) {
	std::cout << _WHITE  _BOLD "gonna read on fd " << c->fd << "\n";
	int ret = c->receiveRequest();
	std::cout << "receiveRequest returned " << ret << '\n';
	std::cout << "c->req is " << c->req << "\n";
	if (ft::checkIfEnded(c->req)) {
		Mutex::Guard	WriteBakGuard(conn->writebakmutex);
		FD_SET(c->fd, &writeFdsBak);
		FD_SET(c->fd, &writeFds);
		std::cout << c->fd << " done reading on " << c->fd << "\n";
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
		std::cout << _RED << response << _END "\n";
		response.clear();
		c->reset();
	} catch (std::exception& e) {
		std::cout << _RED _BOLD "setting client " << c->fd << " to connection closed because responsehandler threw exception\n" _END;
		c->open = false;
	}
	{
		Mutex::Guard	WriteBakGuard(conn->writebakmutex);
		FD_CLR(c->fd, &writeFdsBak);
	}
	return true;
}

void Worker::giveTask(std::pair<std::string, Client *> NewTask) {
	Mutex::Guard	QMutexGuard(this->Qmutex);
	this->WorkerTaskQueue.push(NewTask);
}

void Worker::CommunicateWithConnection(int clientfd, bool DeletionNecessary) {
	std::cout << "Comm with Conn, clientfd is " << clientfd << ", deletion? " << DeletionNecessary << "\n";
	Mutex::Guard	HandleGuard(conn->cHandleMut);
	this->conn->ClientsBeingHandled.erase(clientfd);

	if (DeletionNecessary) {
		Mutex::Guard	DelGuard(conn->cDelMut);
		this->conn->ClientsToBeDeleted.insert(clientfd);
	}
	std::cout << "Worker is done comming with conn\n";
}
