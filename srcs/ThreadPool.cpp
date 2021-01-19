//
// Created by peerdb on 15-01-21.
//

#include "ThreadPool.hpp"
#include "Connection.hpp"
#include <unistd.h>
#include <pthread.h>

ThreadPool::ThreadPool(size_t num, Connection *conn) : numThreads(num), workerThreads(), workers(), conn(conn) {
	this->setupfn();
}

void ThreadPool::setupfn() {
	this->workerThreads = new pthread_t [numThreads];

	for (size_t i = 0; i < numThreads; i++) {
		workers.push_back(new Worker(i, this->conn));
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
	for (std::vector<Worker*>::iterator it = workers.begin(); it != workers.end(); it++) {
		delete *it;
		std::cout << "after deleting worker *it\n";
	}
	delete[] this->workerThreads;
	std::cout << "threadpool is done!\n";
}

void ThreadPool::giveTasksToWorker() {
	while (!this->ThreadPoolTaskQueue.empty()) {
		std::pair<std::string, Client*> Task = ThreadPoolTaskQueue.front();
		std::cout << "we have " << this->numThreads << " workers\n";
		int index = this->findLaziestWorkerIndex();
		std::cout << "laziest mofo is number " << index << "\n";
		Task.second->TaskInProgress = true;

		this->workers[index]->giveTask(Task);
		std::cout << _WHITE "Gave a " << Task.first << " task to worker #" << index << "\n" _END;
		ThreadPoolTaskQueue.pop();
	}
}

int ThreadPool::findLaziestWorkerIndex() {
	size_t	lowest = __INT16_MAX__,
			index = 0,
			tmp;

	for (size_t i = 0; i < this->numThreads; i++) {
		Mutex::Guard<TaskQueue>	WorkerQGuard(this->workers[i]->Qmutex, "laziestworker");
		tmp = this->workers[i]->WorkerTaskQueue.size();
		if (tmp < lowest) {
			lowest = tmp;
			index = i;
		}
	}
	return index;
}

void ThreadPool::joinThreads() {
	for (size_t i = 0; i < numThreads; i++) {
		{
			Mutex::Guard<bool>	WorkerLifeGuard(this->workers[i]->Life, "WorkerLife in joinThreads()");
			this->workers[i]->alive = false;
		}
		std::cout << "Waiting for thread/worker #" << i << " to join.\n";
		int joinret = pthread_join(workerThreads[i], 0);
		std::cout << "joined thread, return is " << joinret << "\n";
	}
	std::cerr << "All threads have been joined\n";
}

void ThreadPool::AddTaskToQueue(const std::pair<std::string, Client *>& NewTask) {
//	this->conn->ConnMutex.lock();
	Mutex::Guard<std::set<int> >	HandleGuard(conn->cHandleMut);
	Mutex::Guard<std::set<int> >	DelGuard(conn->cDelMut);

	if (this->conn->ClientsBeingHandled.count(NewTask.second->fd) == 0 && this->conn->ClientsToBeDeleted.count(NewTask.second->fd) == 0) {
		this->ThreadPoolTaskQueue.push(NewTask);
		this->conn->ClientsBeingHandled.insert(NewTask.second->fd);
		std::cout << "I pushed new " << NewTask.first << " Task to the queue for client " << NewTask.second->fd << "\n";
	} else
		std::cout << _RED "\tError trying to push task to queue\n" _END;
	std::cout << "WorkerTaskQueue has size: " << ThreadPoolTaskQueue.size() << "\n";

}
