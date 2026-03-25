/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:22:23 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/25 14:02:49 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

void	send_ping(t_ping *ping)
{
	AUTO_LOG;

	if (sendto(ping->sockfd, ping->packet, ping->packet_len, 0, ping->addr_info->ai_addr, ping->addr_info->ai_addrlen) <= 0)
	{
		printf(RED "%s: sendto: Failed to send ping packet.\n" RESET, ping->program_name);
		g_is_running = false;
	}

	free(ping->packet);
	ping->packet = NULL;
	return ;
}

int	create_icmp_socket(t_ping *ping)
{
	AUTO_LOG;

	// Create the socket
	if (ping->is_root) // If the user is root, create a raw socket
		ping->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	else // If the user is not root, create a DGRAM socket
	{
		printf(RED "%s: socket: Operation not permitted. Raw sockets require root privileges.\n" RESET, ping->program_name);
		printf(RED "%s: socket: Creating a DGRAM socket instead.\n" RESET, ping->program_name);
		ping->sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
	}

	// Check if the socket was created successfully
	if (ping->sockfd < 0)
		return (close(ping->sockfd),
				printf(RED "%s: socket: Failed to create socket.\n sockfd: %d" RESET, ping->program_name, ping->sockfd),
				EXIT_FAILURE);
	return (EXIT_SUCCESS);
}

int	resolve_hostname(t_ping *ping)
{
	AUTO_LOG;

	struct addrinfo	hints;
	struct addrinfo	*res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	if (getaddrinfo(ping->hostname, NULL, &hints, &res) != 0)
	{
		printf(RED "%s: getaddrinfo: Failed to resolve hostname." RESET, ping->program_name);
		freeaddrinfo(res);	// free
		res = NULL;			// free
		return (EXIT_FAILURE);
	}
	ping->addr_info = res;
	print_addr_info(ping);
	return (EXIT_SUCCESS);
}
