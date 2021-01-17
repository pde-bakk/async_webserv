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
	std::queue<std::pair<std::string, Client*> >	TaskQueue;
	Mutex		Qmutex,
				Life;

public:
	friend class Connection;
	friend class ThreadPool;
	Worker();
	virtual ~Worker();

	void	IOruntime(int index, Connection*);
	void	handleClientRequest(Client* c);
	void	handleClientResponse(Client* c);

	void	giveTask(std::pair<std::string, Client*>& NewTask);
};


#endif //WEBSERV_WORKER_HPP
