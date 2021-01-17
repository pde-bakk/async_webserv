//
// Created by peerdb on 15-01-21.
//

#include "ThreadPool.hpp"
#include "Connection.hpp"
#include <unistd.h>
#include <pthread.h>

ThreadPool::ThreadPool(size_t num) : numThreads(num), workerThreads(), workers(), worker_messages(), conn() {
	this->setupfn();
}

ThreadPool::ThreadPool(size_t num, Connection *conn) : numThreads(num), workerThreads(), workers(), worker_messages(), conn(conn) {
	this->setupfn();
}

void ThreadPool::setupfn() {
	this->workerThreads = new pthread_t [numThreads];
	this->worker_messages = new msg [numThreads];
	this->workers = new Worker [numThreads];

	for (size_t i = 0; i < numThreads; i++) {
		this->worker_messages[i].index = i;
		this->worker_messages[i].workerArray = this->workers;
		this->worker_messages[i].conn = this->conn;
	}

	for (size_t i = 0; i < numThreads; i++) {
		if (pthread_create(&workerThreads[i], NULL, worker_thread_function, &worker_messages[i]) != 0)
			throw std::runtime_error("creating thread failed");
	}
}

void *ThreadPool::worker_thread_function(void *param) {
	msg* message = static_cast<msg *>(param);
	(message->workerArray)[message->index].IOruntime(message->index, message->conn);
	return NULL;
}

ThreadPool::~ThreadPool() {
	std::cout << _GREEN "threadpools dtor" << "\n" _END;
	delete[] this->worker_messages;
	delete[] this->workerThreads;
	delete[] this->workers;
	std::cout << "threadpool is done!\n";
}

void ThreadPool::giveTasksToWorker(std::queue<std::pair<std::string, Client*> >& Queue) {
	while (!Queue.empty()) {
		std::pair<std::string, Client*> Task = Queue.front();
		if (this->conn->TaskSet.count(Task.second->fd)) {
			Queue.pop();
			continue;
		}
		int index = this->findLaziestWorkerIndex();
		Task.second->TaskInProgress = true;

		this->workers[index].giveTask(Task);
		this->conn->TaskSet.insert(Task.second->fd);
		std::cout << _WHITE "Gave a " << Task.first << " task to worker #" << index << "\n" _END;
		Queue.pop();
	}
	std::cout << _PURPLE "Done giving tasks to worker, back to select!\n";
	usleep(50000);
}

int ThreadPool::findLaziestWorkerIndex() {
	size_t	lowest = __INT16_MAX__,
			index = 0,
			tmp;

	for (size_t i = 0; i < this->numThreads; i++) {
		this->workers[i].Qmutex.lock();
		tmp = this->workers[i].TaskQueue.size();
		this->workers[i].Qmutex.unlock();
		if (lowest > tmp) {
			lowest = tmp;
			index = i;
		}
	}
	return index;
}

void ThreadPool::clear() {
	for (size_t i = 0; i < numThreads; i++) {
		std::cout << "i is " << i << "\n";
		workers[i].Life.lock();
		workers[i].alive = false;
		workers[i].Life.unlock();
		std::cout << "Waiting for thread/worker #" << i << " to join.\n";
		int joinret = pthread_join(workerThreads[i], 0);
		std::cout << "joined thread, return is " << joinret << "\n";
	}
	std::cerr << "All threads have been joined\n";
}
