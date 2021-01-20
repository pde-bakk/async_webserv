//
// Created by peerdb on 15-01-21.
//

#include "ThreadPool.hpp"
#include "Connection.hpp"
#include <unistd.h>
#include <pthread.h>

ThreadPool::ThreadPool(size_t num, Connection *conn) : numThreads(num), workerThreads(), workers(), conn(conn), TaskQ(NULL) {
	this->setupfn();
}

void ThreadPool::setupfn() {
	this->workerThreads = new pthread_t [numThreads];
	this->TaskQ = new ThreadSafeQueue<Task*>;

	for (size_t i = 0; i < numThreads; i++) {
		workers.push_back(new Worker(i, this->conn, this->TaskQ));
		if (pthread_create(&workerThreads[i], NULL, worker_thread_function, workers[i]) != 0)
			throw std::runtime_error("creating thread failed");
	}
}

void*	ThreadPool::worker_thread_function(void* param) {
	Worker* worker = static_cast<Worker*>(param);
	worker->IOruntime();
	return NULL;
}

ThreadPool::~ThreadPool() {
	std::cout << _GREEN "threadpools dtor" << "\n" _END;
	this->joinThreads();
	for (std::vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++) {
		delete *it;
		*it = NULL;
		std::cout << "after deleting worker *it\n";
	}
	delete[] this->workerThreads;
	this->workerThreads = NULL;
	delete	this->TaskQ;
	this->TaskQ = NULL;
	std::cout << "threadpool is done!\n";
}

void ThreadPool::joinThreads() {
	for (size_t i = 0; i < numThreads; i++) {
		{
			Mutex::Guard<bool>	WorkerLifeGuard(this->workers[i]->Life);
			this->workers[i]->alive = false;
		}
		std::cout << "Waiting for thread/worker #" << i << " to join.\n";
		int joinret = pthread_join(workerThreads[i], 0);
		std::cout << "joined thread, return is " << joinret << "\n";
	}
	std::cout << "All threads have been joined\n";
}

void ThreadPool::AddTaskToQueue(std::pair<std::string, Client*>* NewTask) {
	{
		Mutex::Guard<std::map<int, Status> >	HandleGuard(conn->cHandleMut);
		if (this->conn->ClientsBeingHandled.count(NewTask->second->fd) == 1) {
			delete NewTask;
			return;
		}
		this->conn->ClientsBeingHandled[NewTask->second->fd] = IN_PROGRESS;
	}
	this->TaskQ->push(NewTask);
}
