//
// Created by peerdb on 10-12-20.
//
#include "Enums.hpp"


std::string	headerTypeAsString(const headerType& header) {
	switch (header) {
		case ACCEPT_CHARSET:
			return "ACCEPT_CHARSET";
		case ACCEPT_LANGUAGE:
			return "ACCEPT_LANGUAGE";
		case ALLOW:
			return "ALLOW";
		case AUTHORIZATION:
			return "AUTHORIZATION";
		case CONNECTION:
			return "CONNECTION";
		case CONTENT_LANGUAGE:
			return "CONTENT_LANGUAGE";
		case CONTENT_LENGTH:
			return "CONTENT_LENGTH";
		case CONTENT_LOCATION:
			return "CONTENT_LOCATION";
		case CONTENT_TYPE:
			return "CONTENT_TYPE";
		case DATE:
			return "DATE";
		case HOST:
			return "HOST";
		case LAST_MODIFIED:
			return "LAST_MODIFIED";
		case LOCATION:
			return "LOCATION";
		case REFERER:
			return "REFERER";
		case REMOTE_USER:
			return "REMOTE_USER";
		case RETRY_AFTER:
			return "RETRY_AFTER";
		case SERVER:
			return "SERVER";
		case TRANSFER_ENCODING:
			return "TRANSFER_ENCODING";
		case USER_AGENT:
			return "USER_AGENT";
		case WWW_AUTHENTICATE:
			return "WWW_AUTHENTICATE";
		default:
			return "INVALID HEADER";
	}
}

std::string methodAsString(const e_method& in) {
	switch (in) {
		case GET:
			return "GET";
		case HEAD:
			return "HEAD";
		case POST:
			return "POST";
		case PUT:
			return "PUT";
		default:
			return "INVALID METHOD";
	}
}
