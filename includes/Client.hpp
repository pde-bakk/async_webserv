//
// Created by peerdb on 13-01-21.
//

#ifndef WEBSERV_CLIENT_HPP
#define WEBSERV_CLIENT_HPP
#include	<arpa/inet.h>
#include	<iostream>
#include	"RequestParser.hpp"
#include	"Colours.hpp"
#include    "Mutex.hpp"

# ifndef CONNECTION_LOGS
#  define CONNECTION_LOGS 0
# endif
#ifndef BONUS
# define BONUS 1
#endif
#define	BUFLEN 8192

class Server;
struct Client {
	Server* parent;
	int		fd,
			port;
	bool	open;
	struct sockaddr_in addr;
	socklen_t size;
	std::string req,
				host,
				ipaddress;
	time_t	lastRequest;
	request_s	parsedRequest;
	Mutex	mut,
			CheckMut;
	bool	TaskInProgress,
			DoneReading;

	explicit Client(Server* x);
	Client(const Client& x);
	Client&	operator=(const Client& x);
	~Client();
	int		receiveRequest();
	void	resetTimeout();
	void 	sendReply(const char* msg, request_s& request) const;
	void	checkTimeout();
	time_t	getTimeDiff() const;
	void	reset();
	static void	breakOnSIGPIPE(int);

private:
	Client();
};

#endif //WEBSERV_CLIENT_HPP
