/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   libftGnl.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pde-bakk <pde-bakk@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/07 15:37:16 by pde-bakk      #+#    #+#                 */
/*   Updated: 2021/01/14 16:07:06 by tuperera      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LIBFTGNL_HPP
# define LIBFTGNL_HPP
#include <vector>
#include "Server.hpp"
# include "../getnextline/get_next_line.hpp"
extern "C" {
	# include "../libft/libft.h"
}

namespace ft {
	int							is_first_char(std::string str, char find = '#');
	void						get_key_value(std::string &str, std::string &key, std::string& value, const char* delim = " \t\n", const char* end = "\n\r#;");
	std::vector<std::string>	split(const std::string& s, const std::string& delim);
	std::string					getextension(const std::string &uri);
	std::string					inttostring(int a);
	void						stringtoupper(std::string& str);
	void 						trimstring(std::string& str, const char* totrim = " ");
	int						 	findNthOccur(std::string str, char ch, int N);
	time_t						getTime();
	bool						checkIfEnded(const std::string& request);
}

#endif
