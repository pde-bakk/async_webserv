/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Location.hpp                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pde-bakk <pde-bakk@student.codam.nl>         +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/10/08 14:26:27 by pde-bakk      #+#    #+#                 */
/*   Updated: 2020/10/15 18:26:22 by pde-bakk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <map>
# include <vector>
# include "Enums.hpp"

class Location {
public:
	friend class Server;
	//coplien form
	Location();
	explicit Location(std::string& );
	virtual ~Location();
	Location(const Location& );
	Location&	operator=(const Location& );

private:		//setters
		void	setroot(const std::string& );
		void	setautoindex(const std::string& );
		void	setallow_method(const std::string& );
		void	setindex(const std::string& );
		void	setcgiallowedextensions(const std::string& );
		void	seterrorpage(const std::string& );
		void	setmaxbody(const std::string& );
		void	setdefaultcgipath(const std::string& );
		void	setphpcgipath(const std::string& );
		void		setauth_basic_realm(const std::string& );
		void		sethtpasswdpath(const std::string& );


public:			//getters
		std::string					getroot() const;
		std::string					getautoindex() const;
		std::string					getlocationmatch() const;
		std::string					getallowedmethods() const;
		std::string					getindex() const;
		std::vector<std::string>	getindexes() const;
		std::vector<std::string>	getcgiallowedextensions() const;
		std::string					geterrorpage() const;
		std::string					getdefaultcgipath() const;
		long unsigned int			getmaxbody() const;
		std::string					getphpcgipath() const;
		std::string 				getauthbasicrealm() const;
		std::string					gethtpasswdpath() const;


		void	setup(int );
		bool	checkifMethodAllowed(const e_method& meth) const;

		bool	getmatch(const std::string& username, const std::string& passwd);
		bool	isExtensionAllowed(const std::string& uri) const;

private:
	std::string					_root,
								_autoindex,
								_location_match,
								_error_page,
								_auth_basic_realm,
								_htpasswd_path;
	std::vector<std::string>	_indexes,
								_cgi_allowed_extensions;
	std::vector<e_method>		_allow_method;
	long unsigned int			_maxBody;
	std::string					_default_cgi_path;
	std::string					_php_cgi;
	std::map<std::string, std::string>	_loginfo;
};

std::ostream&	operator<<(std::ostream& o, const Location& x);

#endif
