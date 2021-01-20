//
// Created by peerdb on 02-12-20.
//

#ifndef WEBSERV_CGI_HPP
#define WEBSERV_CGI_HPP
#include <map>
#include <string>
#include "RequestParser.hpp"

class Cgi {
public:
	Cgi();
	Cgi(const Cgi&);
	Cgi&	operator=(const Cgi&);
	~Cgi();

	int		run_cgi(request_s&, std::string& scriptpath, const std::string& OriginalUri);

private:
	void	populate_map(request_s&, const std::string&, bool redirect_status, std::string& scriptpath);
	void	map_to_env(request_s& request);
	void	clear_env();

	std::map<std::string, std::string>	_m;
	char**								_env;
};

#endif //WEBSERV_CGI_HPP
