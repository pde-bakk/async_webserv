/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Connection.hpp                                     :+:    :+:            */
/*                                                     +:+                    */
/*   By: sam <sam@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/03 15:26:41 by sam           #+#    #+#                 */
/*   Updated: 2020/11/28 20:05:15 by tuperera      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include	"Server.hpp"
#include	"Worker.hpp"
#include	"ThreadPool.hpp"
#include	<set>
#include	<utility>
#include 	<queue>

extern fd_set	readFds,
				writeFds,
				readFdsBak,
				writeFdsBak;

class Connection {
	int _socketFd;
	std::set<int>	_allConnections;
	std::vector<Server*> _servers;
	char* _configPath;
#if BONUS != 0
	size_t	worker_amount;
	ThreadPool*	threadPool;
	Mutex		readmutex,
				readbakmutex,
				writemutex,
				writebakmutex,
				ConnMutex;
	std::queue<std::pair<std::string, Client*> >	TaskQueue;
	std::set<int>	TaskSet;
	std::set<int>	DeleteClients;
#endif
	Connection();
public:
	friend class Worker;
	friend class ThreadPool;
	explicit Connection(char* configPath);
	Connection(const Connection &obj);
	Connection& operator= (const Connection &obj);
	virtual ~Connection();

	void startListening();

	void startServer();
	void loadConfiguration();
	void parsefile();
	void handleCLI();
	void stopServer();
	static void signalServer(int n);
	int		getMaxFD();
	void	transferFD_sets();
	void	checkServer(Server* s);

	void	receiveRequest(Client* c);
	void	handleResponse(Client* c);
	bool	manageClient(Client* c);
	void	deleteTimedOutClients();

};

#endif //CONNECTION_HPP
