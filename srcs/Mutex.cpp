//
// Created by peerdb on 16-01-21.
//

#include "Mutex.hpp"
#include <stdexcept>
#include <cstring>
#include <iostream>

Mutex::Mutex() : _mut(), _type() {
	int ret = pthread_mutex_init(&_mut, 0);
	if (ret != 0)
		error("initialize", ret);
//	debug("initialize");
}

Mutex::Mutex(const char* type) : _mut() {
	this->_type = type;
	int ret = pthread_mutex_init(&_mut, 0);
	if (ret != 0)
		error("initialize", ret);
//	debug("initialize");
}

Mutex::~Mutex() {
	int ret = pthread_mutex_destroy(&_mut);
	if (ret != 0)
		error("destroy", ret);
//	debug("destroy");
}

void Mutex::lock() {
	int ret = pthread_mutex_lock(&_mut);
	if (ret != 0)
		error("lock", ret);
//	debug("lock");
}

void Mutex::unlock() {
	int ret = pthread_mutex_unlock(&_mut);
	if (ret != 0)
		error("unlock", ret);
//	debug("unlock");
}

void Mutex::error(const std::string& operation, int& opcode) {
	std::cerr << "Couldn't " + operation + " my " + _type + " mutex because: " + strerror(opcode) << std::endl;
	throw std::runtime_error("Couldn't " + operation + " my " + _type + " mutex because: " + strerror(opcode));
}

void Mutex::debug(const std::string &operation) {
	std::cerr << "I " + operation + " my " + _type + " mutex.\n";
}

Mutex &Mutex::operator=(const Mutex &x) {
	if (this != &x) {
		_type = x._type;
	}
	return *this;
}

Mutex::Mutex(const Mutex &x) : _mut(), _type() {
	*this = x;
}
