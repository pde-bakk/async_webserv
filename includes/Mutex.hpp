//
// Created by peerdb on 16-01-21.
//

#ifndef WEBSERV_MUTEX_HPP
#define WEBSERV_MUTEX_HPP
#include <pthread.h>
#include <string>

class Mutex {
	pthread_mutex_t	_mut;
	std::string		_type;

	void	error(const std::string& operation, int& opcode);
	void	debug(const std::string& operation);
public:
	Mutex();
	explicit Mutex(const char*);
	Mutex(const Mutex& x);
	Mutex& operator=(const Mutex& x);
	virtual ~Mutex();

	void	lock();
	void 	unlock();

};


#endif //WEBSERV_MUTEX_HPP
