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
# define CONNECTION_HPP
# include	"Server.hpp"
# if BONUS
#  include	"Worker.hpp"
#  include	"ThreadPool.hpp"
# endif
# include	<set>
# include	<utility>
# include 	<queue>
# include 	"Enums.hpp"

extern fd_set	readFds,
				writeFds,
				readFdsBak,
				writeFdsBak;

class Connection {
	Connection();
	int _socketFd;
	std::set<int>	_allConnections;
	std::vector<Server*> _servers;
	char*	_configPath;
	bool	alive;
#if BONUS
	size_t	worker_amount;
	ThreadPool*	threadPool;
	Mutex::Mutex<fd_set>	readmutex,
							readbakmutex,
							writemutex,
							writebakmutex;
	std::map<int, Status>					ClientsBeingHandled;
	Mutex::Mutex<std::map<int, Status> >	cHandleMut;
	Mutex::Mutex<bool>						AliveMut;
public:
	friend class Worker;
	friend class ThreadPool;
	void	multiThreadedListening();
	void	transferFD_sets();
	void	select();
	void	checkServer(Server* s);
	void	receiveRequest(Client* c);
	void	handleResponse(Client* c);
	void	deleteTimedOutClients();
#endif
public:
	friend struct Client;
	explicit Connection(char* configPath);
	Connection(const Connection &obj);
	Connection& operator= (const Connection &obj);
	virtual ~Connection();

	void		startListening();

	void		startServer();
	void		loadConfiguration();
	void		parsefile();
	void		handleCLI();
	void		stopServer();
	int			getMaxFD();
	static void signalServer(int n);
};

#endif //CONNECTION_HPP
