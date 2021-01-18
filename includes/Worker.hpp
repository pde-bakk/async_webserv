//
// Created by peerdb on 15-01-21.
//

#ifndef WEBSERV_WORKER_HPP
#define WEBSERV_WORKER_HPP
#include "Client.hpp"
#include <string>
#include <pthread.h>
#include <queue>

class Connection;

class Worker {
	int			id;
	bool 		alive;
	Connection*	conn;
	std::queue<std::pair<std::string, Client*> >	WorkerTaskQueue;
	Mutex::Mutex	Qmutex;
	Mutex::Mutex	Life;

public:
	friend class Connection;
	friend class ThreadPool;
	Worker();
	Worker(int id, Connection* connection);
	virtual ~Worker();

	void	IOruntime();
	bool 	handleClientRequest(Client* c);
	bool 	handleClientResponse(Client* c);

	void	CommunicateWithConnection(int clientfd, bool deletion);

	void	giveTask(std::pair<std::string, Client*> NewTask);
};


#endif //WEBSERV_WORKER_HPP
