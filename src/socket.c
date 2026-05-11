/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:10:54 by ldalmass          #+#    #+#             */
/*   Updated: 2026/04/29 14:16:10 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"
#include <stddef.h>
#include <stdint.h>

char *transform_raw_ip_to_string_ip(const unsigned int ip)
{
	AUTO_LOG;

	char str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ip, str, INET_ADDRSTRLEN);

	LOG(DEBUG YELLOW "%s" RESET, str);
	return (strdup(str));
}

float deserialize_icmp_packet(t_tr *tr, struct timeval start)
{
	AUTO_LOG;

	uint8_t		*buffer = NULL;
	size_t		buffer_size = tr->packet_len;
	t_replies	*new_reply_node = NULL;
	t_replies	**tail = &tr->replies;

	// Listen for the echo reply
	buffer = calloc(1, buffer_size);
	if (!buffer)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for ICMP packet buffer.\n" RESET, tr->program_name);
		g_is_running = false;
		return (-1.0);
	}

	struct sockaddr_in	src;
	socklen_t			src_len = sizeof(src);
	// if (recv(tr->sockfd, buffer, buffer_size, 0) < 0)
	if (recvfrom(tr->sockfd, buffer, buffer_size, 0, (struct sockaddr *)&src, &src_len) < 0)
	{
		free(buffer);
		LOG(RED "%s: recvfrom: Failed to receive ICMP packet.\n" RESET, tr->program_name);
		return (-1.0);
	}

	// Traverse the linked list to find the last node
	while (*tail) tail = &(*tail)->next;
	new_reply_node = calloc(1, sizeof(t_replies));
	if (!new_reply_node)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for echo reply.\n" RESET, tr->program_name);
		g_is_running = false;
		free(buffer);
		return (-1.0);
	}
	// Fill in additionnal information about the echo reply in the new node
	new_reply_node->reply = *((t_icmp_header *)(buffer + 20));
	// new_reply_node->offset = 20;
	new_reply_node->length = tr->packet_len;
	new_reply_node->reversed_ip = src.sin_addr.s_addr;
	new_reply_node->reversed_ip_str = transform_raw_ip_to_string_ip(src.sin_addr.s_addr);

	// Calculate the elapsed time in seconds
	struct timeval end;
	gettimeofday(&end, NULL);
	uint64_t elapsed_usec = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	new_reply_node->elapsed_time_in_usec = elapsed_usec;
	new_reply_node->elapsed_time_in_ms = elapsed_usec / 1000.0;
	new_reply_node->elapsed_time_in_seconds = elapsed_usec / 1000000.0;
	new_reply_node->reversed_ttl = (tr->is_root) ? ((struct ip *)buffer)->ip_ttl : 0;
	LOG(DEBUG "Elapsed time: %.2f s " RESET, new_reply_node->elapsed_time_in_seconds);
	LOG(DEBUG "Elapsed time: %.2f ms" RESET, new_reply_node->elapsed_time_in_ms);
	LOG(DEBUG "Elapsed time: %.2f us" RESET, new_reply_node->elapsed_time_in_usec);

	// Add the reversed DNS string to the new node if we can
	if (getnameinfo((struct sockaddr *)tr->addr_info->ai_addr, tr->addr_info->ai_addrlen, new_reply_node->reversed_dns_str, NI_MAXHOST, NULL, 0, NI_NAMEREQD) != 0)
		new_reply_node->reversed_dns_str[0] = '\0';

	// Apply the new node to the end of the linked list
	*tail = new_reply_node;
	free(buffer);

	return (new_reply_node->elapsed_time_in_ms);
}

void serialize_icmp_packet(t_tr *tr)
{
	AUTO_LOG;

	size_t i = 0;
	uint8_t *buf = (uint8_t *)&tr->icmp_packet;

	tr->packet = calloc(tr->packet_len, sizeof(uint8_t));
	if (!tr->packet)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for ping packet.\n" RESET, tr->program_name);
		g_is_running = false;
		return;
	}

	// Serialize icmp header
	while (i < sizeof(tr->icmp_packet))
	{
		tr->packet[i] = *((uint8_t *)buf);
		buf++;
		i++;
	}
	return;
}

void send_packet(t_tr *tr)
{
	AUTO_LOG;

	if (sendto(tr->sockfd, tr->packet, tr->packet_len, 0, tr->addr_info->ai_addr, tr->addr_info->ai_addrlen) <= 0)
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
		tr->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	else // If the user is not root, refuse to continue
	{
		printf(RED "%s: socket: Operation not permitted. Raw sockets require root privileges.\n" RESET, tr->program_name);
		printf(MAGENTA "Usage: sudo %s <hostname> [options]\n" RESET, tr->program_name);
		return (EXIT_FAILURE);
	}

	// Check if the socket was created successfully
	if (tr->sockfd < 0)
		return (close(tr->sockfd),
			printf(RED "%s: socket: Failed to create socket.\n sockfd: %d" RESET, tr->program_name, tr->sockfd),
			EXIT_FAILURE);

	// Set the socket timeout for receiving packets and being non-blocking
	struct timeval tv;
	// tv.tv_sec = tr->response_timeout_for_each_probe;
	// tv.tv_usec = 0;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	setsockopt(tr->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)); // set socket to wait x seconds for a response
	setsockopt(tr->sockfd, IPPROTO_IP, IP_TTL, &tr->ttl, sizeof(tr->ttl)); // set the TTL
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
