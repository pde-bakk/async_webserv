/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   ft_memmove.c                                       :+:    :+:            */
/*                                                     +:+                    */
/*   By: pde-bakk <marvin@codam.nl>                   +#+                     */
/*                                                   +#+                      */
/*   Created: 2019/11/14 18:31:41 by pde-bakk       #+#    #+#                */
/*   Updated: 2019/11/15 17:14:06 by pde-bakk      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "libft.h"

void	*ft_memmove(void *dst, const void *src, size_t len)
{
	char		*d;
	const char	*s;
	size_t		i;

	i = 0;
	d = dst;
	s = src;
	if (dst == src)
		return (dst);
	if (d < s)
		while (i < len)
		{
			d[i] = s[i];
			i++;
		}
	else
		while (len)
		{
			d[len - 1] = s[len - 1];
			len--;
		}
	return (dst);
}
