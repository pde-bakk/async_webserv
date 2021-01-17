//
// Created by peerdb on 15-01-21.
//

#ifndef WEBSERV_THREADPOOL_HPP
#define WEBSERV_THREADPOOL_HPP
#include "Worker.hpp"
#include <stdexcept>

struct msg {
	int index;
	Worker* workerArray;
	Connection*	conn;
};

class ThreadPool {
	size_t 		numThreads;
	pthread_t*	workerThreads;
	Worker*		workers;
	msg*		worker_messages;
	Connection*	conn;

	static void*	worker_thread_function(void*);
	explicit ThreadPool(size_t num);
public:
	ThreadPool(size_t num, Connection*);
	void setupfn();
	virtual ~ThreadPool();
	void	giveTasksToWorker(std::queue<std::pair<std::string, Client*> >& );
	int		findLaziestWorkerIndex();
	void	clear();
};


#endif //WEBSERV_THREADPOOL_HPP
