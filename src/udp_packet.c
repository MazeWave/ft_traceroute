/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   udp_packet.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/24 17:11:14 by ldalmass          #+#    #+#             */
/*   Updated: 2026/08/24 17:11:15 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"
#include <netinet/in.h>
#include <stdint.h>

void	build_udp_packet(t_tr *tr)
{
	AUTO_LOG;

	tr->udp_packet.source_port = htons(0);
	tr->udp_packet.destination_port = htons(0);
	tr->udp_packet.length = htons(8);
	LOG(GREEN "UDP header initialized" RESET);
	serialize_udp_packet(tr);
	if (!tr->packet) return;
	tr->udp_packet.checksum = calculate_checksum((uint16_t *)tr->packet, tr->packet_len);
	LOG(GREEN "UDP checksum calculated" RESET);
	LOG(BLUE "Filling UDP packet..." RESET);
	memcpy(&tr->packet[0], &tr->udp_packet.source_port, sizeof(uint16_t));
	memcpy(&tr->packet[2], &tr->udp_packet.destination_port, sizeof(uint16_t));
	memcpy(&tr->packet[4], &tr->udp_packet.length, sizeof(uint16_t));
	memcpy(&tr->packet[6], &tr->udp_packet.checksum, sizeof(uint16_t));
	LOG(GREEN "UDP packet filled" RESET);

	return ;
}

void serialize_udp_packet(t_tr *tr)
{
	AUTO_LOG;

	size_t i = 0;
	uint8_t *buffer = (uint8_t *)&tr->udp_packet;

	tr->packet = calloc(tr->packet_len, sizeof(uint8_t));
	if (!tr->packet)
	{
		LOG(RED "%s: calloc: Failed to allocate memory for udp packet.\n" RESET, tr->program_name);
		g_is_running = false;
		return;
	}

	// Serialize icmp header
	while (i < sizeof(tr->udp_packet))
	{
		tr->packet[i] = *((uint8_t *)buffer);
		buffer++;
		i++;
	}
	LOG(GREEN "UDP: packet serialized correctly\n" RESET, tr->program_name);
	return;
}

float deserialize_udp_packet(t_tr *tr, struct timeval start)
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
	if (recvfrom(tr->send_sockfd, buffer, buffer_size, 0, (struct sockaddr *)&src, &src_len) < 0)
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
	// new_reply_node->reversed_ip_str = transform_raw_ip_to_string_ip(src.sin_addr.s_addr);
	new_reply_node->reversed_ip_str = transform_raw_ip_to_string_ip(src.sin_addr);

	// Calculate the elapsed time in seconds
	struct timeval end;
	gettimeofday(&end, NULL);
	uint64_t elapsed_usec = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec);
	new_reply_node->elapsed_time_in_usec = elapsed_usec;
	new_reply_node->elapsed_time_in_ms = elapsed_usec / 1000.0;
	new_reply_node->elapsed_time_in_seconds = elapsed_usec / 1000000.0;
	new_reply_node->reversed_ttl = ((struct ip *)buffer)->ip_ttl;
	// new_reply_node->reversed_dns_str = (tr->is_root) ? ((struct ip *)buffer)->ip_ttl : 0;
	LOG(DEBUG "Elapsed time: %.2f s " RESET, new_reply_node->elapsed_time_in_seconds);
	LOG(DEBUG "Elapsed time: %.2f ms" RESET, new_reply_node->elapsed_time_in_ms);
	LOG(DEBUG "Elapsed time: %.2f us" RESET, new_reply_node->elapsed_time_in_usec);

	// Add the reversed DNS string to the new node if we can
	if (getnameinfo((struct sockaddr *)&src, src_len, new_reply_node->reversed_dns_str, NI_MAXHOST, NULL, 0, NI_NAMEREQD) != 0)
		new_reply_node->reversed_dns_str[0] = '\0';

	// Apply the new node to the end of the linked list
	*tail = new_reply_node;
	free(buffer);

	return (new_reply_node->elapsed_time_in_ms);
}