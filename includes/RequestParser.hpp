/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   RequestParser.hpp                                  :+:    :+:            */
/*                                                     +:+                    */
/*   By: sam <sam@student.codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/06 12:01:32 by sam           #+#    #+#                 */
/*   Updated: 2020/12/15 13:01:05 by tuperera      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUESTPARSER_HPP
#define REQUESTPARSER_HPP

#include <map>
#include <string>
#include "Enums.hpp"

class Server;
class Location;
struct request_s {
	int									status_code;
	e_method							method;
	std::string							uri;
	std::string							cgiparams;
	std::pair<int, int>					version;
	std::map<headerType, std::string>	headers;
	Server*								server;
	Location*							location;
	std::string							body;
	bool								transfer_buffer,
										cgi_ran;
	std::map<std::string, std::string>	env;

	request_s();
	request_s(const request_s& x);
	request_s& operator=(const request_s& x);
	~request_s();
	std::string		MethodToSTring() const;
	void			clear();
};

std::ostream&	operator<<(std::ostream& o, const request_s& r);

class RequestParser {
	int									_status_code;
	e_method							_method;
	std::string							_uri;
	std::pair<int, int>					_version;
	std::map<headerType, std::string>	_headers;
	std::map<std::string, e_method>		_methodMap;
	std::map<std::string, headerType>	_headerMap;
	std::string							_rawRequest;
	std::map<std::string, std::string>	_env;

public:
	friend class ResponseHandler;
	friend class Connection;
	RequestParser();
	virtual ~RequestParser();
	RequestParser(const RequestParser &obj);
	RequestParser&	operator= (const RequestParser &obj);

	request_s		parseRequest(const std::string &req);
	void			parseBody(request_s& req);
	void			AddHeaderToEnv(const std::string& upperHeader, const std::string& value);
	request_s		parseHeadersOnly(const std::string &req);
	void			parseRequestLine();
	void			parseHeaders();
	void			setRawRequest(const std::string& rawRequest);
	void			extractMethod(size_t eoRequestLine, size_t& pos);
	void			extractUri(size_t eoRequestLine, size_t pos, size_t pos2);
	void			extractVersion(size_t eoRequestLine, size_t& pos, size_t &pos2);
	e_method		getMethod() const;

	const std::string&							getUri() const;
	const std::pair<int, int>& 					getVersion() const;
	const std::map<headerType, std::string>&	getHeaders() const;
};


#endif //REQUESTPARSER_HPP
