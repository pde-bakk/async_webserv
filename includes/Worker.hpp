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

typedef std::pair<std::string, Client*>		Task;
typedef std::queue<Task> TaskQueue;

class Worker {
	int			id;
	bool 		alive;
	Connection*	conn;
	TaskQueue	WorkerTaskQueue;
	Mutex::Mutex<TaskQueue>	Qmutex;
	Mutex::Mutex<bool>	Life;

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

	void	giveTask(const Task& NewTask);
};


#endif //WEBSERV_WORKER_HPP
