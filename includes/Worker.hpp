//
// Created by peerdb on 15-01-21.
//

#ifndef WEBSERV_WORKER_HPP
#define WEBSERV_WORKER_HPP
#include "Client.hpp"
#include "ThreadSafeQueue.hpp"
#include <string>
#include <pthread.h>
#include <queue>
#include "Enums.hpp"

class Connection;

class Worker {
	int			id;
	bool 		alive;
	Connection*	conn;
	ThreadSafeQueue<Task*>*		TaskQ;
	Mutex::Mutex<bool>	Life;

public:
	friend class Connection;
	friend class ThreadPool;
	Worker();
	Worker(int id, Connection* connection, ThreadSafeQueue<Task*>* Q);
	virtual ~Worker();

	void	IOruntime();
	Status 	handleClientRequest(Client* c);
	Status 	handleClientResponse(Client* c);

	void	CommunicateWithConnection(int clientfd, Status status);
};


#endif //WEBSERV_WORKER_HPP
