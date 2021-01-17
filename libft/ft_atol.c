/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ft_atol.c                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: samkortekaas <samkortekaas@student.coda      +#+                     */
/*                                                   +#+                      */
/*   Created: 2020/04/29 19:49:58 by samkortekaa   #+#    #+#                 */
/*   Updated: 2020/10/17 16:23:52 by peerdb        ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <limits.h>
#include "libft.h"

long	ft_atol(const char *str)
{
	int		sign;
	long	result;

	result = 0;
	sign = 1;
	while (iswhitespace(*str))
		str++;
	if (*str == '-')
	{
		sign = -1;
		str++;
	}
	else if (*str == '+')
		str++;
	while (*str >= '0' && *str <= '9')
	{
		result = (10 * result) + (*str - '0');
		str++;
	}
	if (result > LONG_MAX && sign == -1)
		return (0);
	else if (result >= LONG_MAX)
		return (-1);
	return (sign * result);
}
