/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp_packet.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:11:14 by ldalmass          #+#    #+#             */
/*   Updated: 2026/04/28 17:11:15 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"

void build_icmp_packet(t_tr *tr)
{
	AUTO_LOG;

	init_icmp_header(tr);
	serialize_icmp_packet(tr);
	if (!tr->packet) return;
	tr->icmp_packet.checksum = calculate_checksum((uint16_t *)tr->packet, tr->packet_len);
	// *(uint16_t *)&tr->packet[2] = tr->icmp_packet.checksum; // this writes both the packet[2] and packet[3]
			tr->packet[0] = tr->icmp_packet.type;
			tr->packet[1] = tr->icmp_packet.code;
	memcpy(&tr->packet[2], &tr->icmp_packet.checksum, sizeof(uint16_t));
	memcpy(&tr->packet[4], &tr->icmp_packet.identifier, sizeof(uint16_t));
	memcpy(&tr->packet[6], &tr->icmp_packet.sequence_number, sizeof(uint16_t));
	return;
}

uint16_t calculate_checksum(void *addr, int count)
{
	AUTO_LOG;

	// Source : https://www.rfc-editor.org/rfc/rfc1071
	/* Compute Internet Checksum for "count" bytes
	*         beginning at location "addr".
	*/
	uint32_t sum = 0;
	uint16_t *temp = (uint16_t *)addr;
	while (count > 1)
	{
		/*  This is the inner loop */
		sum += *temp++;
		count -= 2;
	}
	/*  Add left-over byte, if any */
	if (count > 0)
		sum += *(uint8_t *)temp;
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	LOG(GREEN "Calculated checksum: %u" RESET, (uint16_t)(~sum));
	return ((uint16_t)(~sum));
}

void init_icmp_header(t_tr *tr)
{
	AUTO_LOG;

	static int sequence_number = 0;

	tr->icmp_packet.type = ICMP_ECHO;
	tr->icmp_packet.code = 0;
	tr->icmp_packet.checksum = 0;
	tr->icmp_packet.identifier = getpid() & 0xffff;
	tr->icmp_packet.sequence_number = sequence_number++;

	LOG(GREEN "ICMP header initialized" RESET);
	return;
}

void serialize_icmp_packet(t_tr *tr)
{
	AUTO_LOG;

	size_t i = 0;
	uint8_t *buffer = (uint8_t *)&tr->icmp_packet;

	tr->packet = calloc(tr->packet_len, sizeof(uint8_t));
	if (!tr->packet)
	{
		LOG(RED "%s: calloc: Failed to allocate memory for icmp packet.\n" RESET, tr->program_name);
		g_is_running = false;
		return;
	}

	// Serialize icmp header
	while (i < sizeof(tr->icmp_packet))
	{
		tr->packet[i] = *((uint8_t *)buffer);
		buffer++;
		i++;
	}
	return;
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
	if (recvfrom(tr->recv_sockfd, buffer, buffer_size, 0, (struct sockaddr *)&src, &src_len) < 0)
	{
		free(buffer);
		LOG(RED "%s: recvfrom: Failed to receive ICMP packet." RESET, tr->program_name);
		if (errno == EAGAIN || errno ==  EWOULDBLOCK)
			LOG(YELLOW "recv socket timeout, did not receive response within target response time." RESET);
		else if (errno == EINTR)
			LOG(YELLOW "CTRL+C interrupt detected." RESET);
		else print_errno();
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