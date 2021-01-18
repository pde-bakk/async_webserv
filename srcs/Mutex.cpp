//
// Created by peerdb on 16-01-21.
//

#include "Mutex.hpp"

namespace Mutex {

	Mutex::Mutex() : _mut(), _type() {
		this->assert(pthread_mutex_init(&_mut, 0), "initialize");
	}
	Mutex::Mutex(const char* x) : _mut(), _type(x) {
		this->assert(pthread_mutex_init(&_mut, 0), "typed initialize");
	}
	Mutex::~Mutex() {
		this->assert(pthread_mutex_destroy(&_mut), "destroy");
	}

	void Mutex::lock() {
//		std::cout << "locking " << _type << "\n";
		this->assert(pthread_mutex_lock(&_mut), "lock");
	}
	void Mutex::unlock() {
//		std::cout << "unlocking " << _type << "\n";
		this->assert(pthread_mutex_unlock(&_mut), "unlock");
	}

	void	Mutex::assert(int opcode, const std::string& operation) {
		if (opcode != 0)
			throw std::runtime_error("Couldn't " + operation + " my " + _type + " mutex because '" _RED + strerror(opcode) + _END "\n");
	}

	Guard::Guard(Mutex& m) : _mut(m), _type() {
		_mut.lock();
	}
	Guard::Guard(Mutex& m, const char* x) : _mut(m), _type(x) {
		std::cout << "Waiting to lock mutexGuard " << _type << "\n";
		_mut.lock();
		std::cout << "Locked mutexGuard " << _type << "\n";
	}
	Guard::~Guard() {
		if (_type.empty())
			_mut.unlock();
		else {
			std::cout << "Waiting to unlock mutexGuard " << _type << "\n";
			_mut.unlock();
			std::cout << "Unlocked mutexGuard " << _type << "\n";
		}
	}
}

