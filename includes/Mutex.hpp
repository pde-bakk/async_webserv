//
// Created by peerdb on 16-01-21.
//

#ifndef WEBSERV_MUTEX_HPP
#define WEBSERV_MUTEX_HPP
#include <pthread.h>
#include <string>
#include <stdexcept>
#include "Colours.hpp"
#include <cstring>
#include <iostream>

namespace Mutex {

	class Mutex {
		pthread_mutex_t _mut;
		std::string _type;

		Mutex(const Mutex &x); // coplien
		Mutex &operator=(const Mutex &x); // coplien

		void	assert(int opcode, const std::string& operation);
	public:
		friend class Guard;
		Mutex();
		explicit Mutex(const char *);
		Mutex(char* x);
		virtual ~Mutex();
		void lock();
		void unlock();

	};

	class Guard {
		Guard(); // coplien
		Guard(const Guard &x); // coplien
		Guard &operator=(const Guard &x); // coplien

		Mutex&		_mut;
		std::string	_type;
	public:
		explicit Guard(Mutex& m);
		explicit Guard(Mutex& m, const char*);
		~Guard();
	};

}

#endif //WEBSERV_MUTEX_HPP
