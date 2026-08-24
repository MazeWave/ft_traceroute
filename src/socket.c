/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:10:54 by ldalmass          #+#    #+#             */
/*   Updated: 2026/05/15 15:14:51 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>

// char *transform_raw_ip_to_string_ip(const unsigned int ip)
// {
// 	AUTO_LOG;

// 	// char str[INET_ADDRSTRLEN];
// 	// inet_ntop(AF_INET, &ip, str, INET_ADDRSTRLEN);
// 	char *ip_str = inet_ntoa(ai_addr->sin_addr);
	

// 	LOG(DEBUG YELLOW "%s" RESET, str);
// 	return (strdup(str));
// }

char *transform_raw_ip_to_string_ip(struct in_addr ip)
{
	AUTO_LOG;

	char *ip_str = inet_ntoa(ip);
	LOG(DEBUG YELLOW "%s" RESET, ip_str);
	return (strdup(ip_str));
}

void send_packet(t_tr *tr)
{
	AUTO_LOG;

	if (sendto(tr->send_sockfd, tr->packet, tr->packet_len, 0, tr->addr_info->ai_addr, tr->addr_info->ai_addrlen) <= 0)
	{
		printf(RED "%s: sendto: Failed to send ping packet.\n" RESET, tr->program_name);
		g_is_running = false;
	}

	free(tr->packet);
	tr->packet = NULL;
	return;
}

int create_icmp_socket(t_tr *tr)
{
	AUTO_LOG;

	// Create the socket
	if (tr->is_root) // If the user is root, create a raw socket
		tr->send_sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	else // If the user is not root, refuse to continue
	{
		printf(RED "%s: socket: Operation not permitted. Raw sockets require root privileges.\n" RESET, tr->program_name);
		printf(MAGENTA "Usage: sudo %s <hostname> [options]\n" RESET, tr->program_name);
		return (EXIT_FAILURE);
	}

	// Check if the socket was created successfully
	if (tr->send_sockfd < 0)
		return (close(tr->send_sockfd),
			printf(RED "%s: socket: Failed to create socket.\n sockfd: %d" RESET, tr->program_name, tr->send_sockfd),EXIT_FAILURE);

	// Set the socket timeout for receiving packets and being non-blocking
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000; // 100 ms
	uint32_t	final_ttl = tr->ttl + tr->offset_hop;
	setsockopt(tr->send_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // set socket to wait x seconds/ms for a response
	setsockopt(tr->send_sockfd, IPPROTO_IP, IP_TTL, &final_ttl, sizeof(final_ttl)); // set the TTL
	setsockopt(tr->send_sockfd, IPPROTO_IP, IP_TOS, &tr->tos, sizeof(tr->tos)); // set the TOS
	return (EXIT_SUCCESS);
}

int create_udp_socket(t_tr *tr)
{
	AUTO_LOG;

	// Create the socket
	tr->send_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Check if the socket was created successfully
	if (tr->send_sockfd < 0)
		return (
			printf(RED "%s: socket: Failed to create socket.\n sockfd: %d" RESET, tr->program_name, tr->send_sockfd),EXIT_FAILURE);

	// // Set the socket timeout for receiving packets and being non-blocking
	// struct timeval tv;
	// tv.tv_sec = 0;
	// tv.tv_usec = 100000; // 100 ms
	// uint32_t	final_ttl = tr->ttl + tr->offset_hop;
	// setsockopt(tr->send_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // set socket to wait x seconds/ms for a response
	// setsockopt(tr->send_sockfd, IPPROTO_IP, IP_TTL, &final_ttl, sizeof(final_ttl)); // set the TTL
	// setsockopt(tr->send_sockfd, IPPROTO_IP, IP_TOS, &tr->tos, sizeof(tr->tos)); // set the TOS
	return (EXIT_SUCCESS);
}

int resolve_hostname(t_tr *tr)
{
	AUTO_LOG;

	struct addrinfo hints;
	struct addrinfo *res;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	if (getaddrinfo(tr->hostname, NULL, &hints, &res) != 0)
	{
		printf(RED "%s: getaddrinfo: Failed to resolve hostname." RESET, tr->program_name);
		return (EXIT_FAILURE);
	}
	tr->addr_info = res;
	find_the_ip(tr);
	return (EXIT_SUCCESS);
}
