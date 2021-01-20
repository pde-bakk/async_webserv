//
// Created by peerdb on 15-01-21.
//

#ifndef WEBSERV_THREADPOOL_HPP
#define WEBSERV_THREADPOOL_HPP
#include "Worker.hpp"
#include <stdexcept>

class ThreadPool {
	size_t 		numThreads;
	pthread_t*	workerThreads;
	std::vector<Worker*> workers;
	Connection*	conn;
	ThreadSafeQueue<Task*>*		TaskQ;

	static void*	worker_thread_function(void*);
	ThreadPool();
	ThreadPool(const ThreadPool&);
	ThreadPool& operator=(const ThreadPool&);
public:
	ThreadPool(size_t num, Connection*);
	void setupfn();
	virtual ~ThreadPool();
	void	joinThreads();
	void	AddTaskToQueue(std::pair<std::string, Client*>* NewTask);
};


#endif //WEBSERV_THREADPOOL_HPP
