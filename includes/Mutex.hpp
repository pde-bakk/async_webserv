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

	template <typename T>
	class Mutex {
		T&				_inner_type;
		pthread_mutex_t _mut;
		std::string _type;

		Mutex(const Mutex &x); // coplien
		Mutex &operator=(const Mutex &x); // coplien

		void	assertMut(int opcode, const std::string& operation) {
			if (opcode != 0)
				throw std::runtime_error("Couldn't " + operation + " my " + _type + " mutex because '" _RED + strerror(opcode) + _END "\n");
		}
	public:
		template <typename T2>
		friend class Guard;
		Mutex(T& x) : _inner_type(x), _mut(), _type() { this->assertMut(pthread_mutex_init(&_mut, 0), "initialize"); };
		Mutex(T& x, const char* s) : _inner_type(x), _mut(), _type(s) {
			this->assertMut(pthread_mutex_init(&_mut, 0), "typed initialize"); }
		virtual ~Mutex() { this->assertMut(pthread_mutex_destroy(&_mut), "destroy"); }
		void lock() { this->assertMut(pthread_mutex_lock(&_mut), "lock"); }
		void unlock() { this->assertMut(pthread_mutex_unlock(&_mut), "unlock"); }
	};

	template <typename T>
	class Guard {
		Guard(); // coplien
		Guard(const Guard &x); // coplien
		Guard &operator=(const Guard &x); // coplien

		Mutex<T>&	_mut;
		std::string	_type;
	public:
		explicit Guard(Mutex<T>& m) : _mut(m), _type() { _mut.lock(); }
		explicit Guard(Mutex<T>& m, const char* s) : _mut(m), _type(s) {
			std::cout << "Waiting to lock mutexGuard " << _type << "\n";
			_mut.lock();
			std::cout << "Locked mutexGuard " << _type << "\n";
		}
		~Guard() {
			if (_type.empty())
			_mut.unlock();
			else {
				std::cout << "Waiting to unlock mutexGuard " << _type << "\n";
				_mut.unlock();
				std::cout << "Unlocked mutexGuard " << _type << "\n";
			}
		}
		T&		get() { return _mut._inner_type; }
		void	set(T x) { _mut._inner_type = x; }
	};

}

#endif //WEBSERV_MUTEX_HPP
