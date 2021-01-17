//
// Created by peerdb on 09-12-20.
//

#include "libftGnl.hpp"
#include <string>
#include <sys/time.h>

namespace ft {

	int		is_first_char(std::string str, char find) {
		int i = 0;
		while (str[i] && iswhitespace(str[i]))
			++i;
		if (str[i] == find)
			return (1);
		return (0);
	}

	//	delim and end are defaulted to use " \t\n" as delim and "\n\r#;" as end
	void	get_key_value(std::string &str, std::string &key, std::string& value, const char* delim, const char* end) {
		size_t kbegin = str.find_first_not_of(delim);
		size_t kend = str.find_first_of(delim, kbegin);
		key = str.substr(kbegin, kend - kbegin);
		size_t vbegin = str.find_first_not_of(delim, kend);
		size_t vend = str.find_first_of(end, vbegin);
		value = str.substr(vbegin, vend - vbegin);
	}

	std::string getextension(const std::string &uri) {
		size_t dotindex = uri.rfind('.');
		if (dotindex == std::string::npos)
			return "";
		return uri.substr(dotindex, uri.find_first_of('/', dotindex) - dotindex);
	}

	std::string inttostring(int n) {
		std::string ss;

		if (n == 0)
			ss = "0";
		while (n) {
			char i = '0' + (n % 10);
			n /= 10;
			ss = i + ss;
		}
		return ss;
	}
	void	stringtoupper(std::string& str) {
		for (size_t i = 0; i < str.length(); ++i) {
			str[i] = ft_toupper(str[i]);
		}
	}

	void	trimstring(std::string& str, const char* totrim) {
		size_t	begin = str.find_first_not_of(totrim),
				end = str.find_last_not_of(totrim);
		if (begin == std::string::npos || end == 0)
			return;
		str = str.substr(begin, end - begin + 1);
	}

	int findNthOccur(std::string str, char ch, int N)
	{
		int occur = 0;

		for (int i = (str.length()); i >= 0; i--) {
			if (str[i] == ch) {
				occur += 1;
			}
			if (occur == N)
				return i;
		}
		return -1;
	}
	
	time_t	getTime() {
		struct timeval tv = {};
		struct timezone tz = {};

		gettimeofday(&tv, &tz);
		return (tv.tv_usec);
	}

	bool checkIfEnded(const std::string& request) {
		size_t chunkedPos;
		size_t encoPos = request.find("Transfer-Encoding:");
		if (encoPos != std::string::npos) {
			chunkedPos = request.find("chunked", encoPos);
		}
		if (encoPos != std::string::npos && chunkedPos != std::string::npos) {
			size_t endSequencePos = request.find("\r\n0\r\n\r\n");
			size_t len = request.length();
			if (endSequencePos != std::string::npos && endSequencePos + 7 == len) {
				return true;
			}
		}
		else {
			size_t endSequencePos = request.find_last_not_of("\r\n\r\n");
			size_t len = request.length();
			if (endSequencePos != std::string::npos && endSequencePos + 5 == len) {
				return true;
			}
		}
		return false;
	}
}
