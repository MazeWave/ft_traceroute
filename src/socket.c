/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/12 15:22:23 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/25 18:2929:1010 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

void	deserialize_icmp_packet(t_ping *ping)
{
	AUTO_LOG;

	size_t		offset = (ping->is_root) ? 20 : 0;
	size_t		buffer_size = ping->packet_len + offset;
	void		*buffer = NULL;
	t_replies	*new_reply_node = NULL;
	t_replies	**tail = &ping->replies;

	// Listen for the echo reply
	buffer = calloc(1, buffer_size);
	if (!buffer)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for ICMP packet buffer.\n" RESET, ping->program_name);
		g_is_running = false;
		return ;
	}
	if (recv(ping->sockfd, buffer, buffer_size, 0) < 0)
	{
		free(buffer);
		LOG(RED "%s: recv: Failed to receive ICMP packet.\n" RESET, ping->program_name);
		return ;
	}

	// Traverse the linked list to find the last node
	while (*tail)
		tail = &(*tail)->next;
	new_reply_node = calloc(1, sizeof(t_replies));
	if (!new_reply_node)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for echo reply.\n" RESET, ping->program_name);
		g_is_running = false;
		free(buffer);
		return ;
	}
	new_reply_node->reply = *((t_icmp_header *)(buffer + offset));
	*tail = new_reply_node;
	free(buffer);
}

void	serialize_icmp_packet(t_ping *ping)
{
	AUTO_LOG;

	size_t		i = 0;
	size_t		len = sizeof(ping->icmp_packet) + ping->payload_length;
	uint8_t		*buf = (uint8_t *)&ping->icmp_packet;
	uint8_t		*payload_data = (uint8_t *)&ping->payload_raw_string;

	ping->packet = calloc(ping->packet_len, sizeof(uint8_t));
	if (!ping->packet)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for ping packet.\n" RESET, ping->program_name);
		g_is_running = false;
		return ;
	}

	// Serialize icmp header
	while (i < sizeof(ping->icmp_packet))
	{
	    ping->packet[i] = *((uint8_t *)buf);
	    buf++;
	    i++;
	}
	// Serialize payload data
	while (i < len)
	{
	    ping->packet[i] = payload_data[i - sizeof(ping->icmp_packet)];
	    i++;
	}
	return ;
}

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
	
	// Set the socket timeout for receiving packets and being non-blocking
	struct timeval	tv;
	tv.tv_sec = ping->interval;
	tv.tv_usec = (ping->interval - tv.tv_sec) * 1000000.0;
	setsockopt(ping->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
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
