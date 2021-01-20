//
// Created by Peer De bakker on 11/15/20.
//

#ifndef WEBSERV_ENUMS_HPP
#define WEBSERV_ENUMS_HPP
#include <string>

enum Status {
	IN_PROGRESS,
	DONE_READING,
	DONE_WRITING
};

std::string StatusToString(const Status& in);

enum headerType { // TODO No ACCEPT_ENCODING???
	ACCEPT_CHARSET,
	ACCEPT_LANGUAGE,
	ALLOW,
	AUTHORIZATION,
	CONNECTION,
	CONTENT_LANGUAGE,
	CONTENT_LENGTH,
	CONTENT_LOCATION,
	CONTENT_TYPE,
	DATE,
	HOST,
	LAST_MODIFIED,
	LOCATION,
	REFERER,
	REMOTE_USER,
	RETRY_AFTER,
	SERVER,
	TRANSFER_ENCODING,
	USER_AGENT,
	WWW_AUTHENTICATE
};

std::string	headerTypeAsString(const headerType& header);

enum e_method {
	GET, // These are mandatory according to rfc's
	HEAD, // These are mandatory according to rfc's
	POST,
	PUT,
	ERROR
};

std::string methodAsString(const e_method&);

#endif //WEBSERV_ENUMS_HPP
