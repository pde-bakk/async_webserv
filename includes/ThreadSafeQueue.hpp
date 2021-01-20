//
// Created by peerdb on 20-01-21.
//

#ifndef WEBSERV_THREADSAFEQUEUE_HPP
#define WEBSERV_THREADSAFEQUEUE_HPP
#include <queue>
#include "Mutex.hpp"

struct Client;

typedef std::pair<std::string, Client*>		Task;
typedef std::queue<Task*> TaskQueue;

template <typename T>
class ThreadSafeQueue {
	std::queue<T>	_data;
	Mutex::Mutex<TaskQueue>	_mut;

public:
	ThreadSafeQueue() : _data(), _mut(_data) { }
	~ThreadSafeQueue() {
		Mutex::Guard<TaskQueue>	QueueGuard(this->_mut);
		while (!this->_data.empty()) {
			delete this->_data.front();
			this->_data.pop();
		}
	}
	void	push(Task* x) {
		Mutex::Guard<TaskQueue>	QueueGuard(this->_mut);
		this->_data.push(x);
	}
	T		pop() {
		Mutex::Guard<TaskQueue>	QueueGuard(this->_mut);
		if (this->_data.empty())
			return NULL;
		T	ret(this->_data.front());
		this->_data.pop();
		return (ret);
	}
	size_t	size() {
		Mutex::Guard<TaskQueue>	QueueGuard(this->_mut);
		return (this->_data.size());
	}
};


#endif //WEBSERV_THREADSAFEQUEUE_HPP
