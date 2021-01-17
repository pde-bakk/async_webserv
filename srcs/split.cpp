/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   split.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: pde-bakk <pde-bakk@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/08 16:11:55 by pde-bakk      #+#    #+#                 */
/*   Updated: 2020/10/17 16:32:30 by peerdb        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>

namespace ft {
	
	std::vector<std::string>	split(const std::string& s, const std::string& delim) {
		size_t start, end = 0;
		std::vector<std::string> vec;

		while (end != std::string::npos) {
			start = s.find_first_not_of(delim, end);
			end = s.find_first_of(delim, start);
			if (end != std::string::npos || start != std::string::npos)
				vec.push_back(s.substr(start, end - start));
		}
		return vec;
	}

}
