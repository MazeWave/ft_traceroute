/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp_packet.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:09:13 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/25 17:2020:4545 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"
#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

void	deserialize_icmp_packet(t_ping *ping)
{
	AUTO_LOG;

	size_t		buffer_size = 0;
	void		*buffer = NULL;
	t_replies	*temp = ping->replies;

	// Create our custom linked list to store the echo replies if needed
	if (!ping->replies)
	{
		ping->replies = calloc(sizeof(t_replies), 1);
		if (!ping->replies)
		{
			printf(RED "%s: malloc: Failed to allocate memory for echo replies linked list.\n" RESET, ping->program_name);
			g_is_running = false;
			return ;
		}
	}

	// Allocate a buffer to receive the packet
	// SOCK_RAW : IP Header + ICMP Header + Payload (+20 bytes for the IP header)
	// SOCK_DGRAM : ICMP Header + Payload
	buffer_size = (ping->is_root) ? ping->packet_len + 20 : ping->packet_len;
	buffer = calloc(buffer_size, 1);
	if (!buffer)
	{
		printf(RED "%s: malloc: Failed to allocate memory for ICMP packet buffer.\n" RESET, ping->program_name);
		g_is_running = false;
		return ;
	}

	// Receive the packet
	if (recv(ping->sockfd, buffer, buffer_size, 0) < 0)
	{
		free(buffer);
		printf(RED "%s: recv: Failed to receive ICMP packet.\n" RESET, ping->program_name);
		return ;
	}

	// Go to last node of the linked list
	while (temp->next)
		temp = temp->next;
	// Allocate memory for the reply
	temp = calloc(sizeof(t_replies), 1);
	if (!temp->reply)
	{
		printf(RED "%s: malloc: Failed to allocate memory for echo reply.\n" RESET, ping->program_name);
		g_is_running = false;
		free(buffer);
		return ;
	}

	// Deserialization step
	temp->reply = calloc(sizeof(t_icmp_header), 1);
	if (!temp->reply)
	{
		printf(RED "%s: malloc: Failed to allocate memory for echo reply header.\n" RESET, ping->program_name);
		g_is_running = false;
		free(buffer);
		return ;
	}
	temp->reply = (t_icmp_header *)(buffer + buffer_size);
	return ;
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
		printf(RED "%s: malloc: Failed to allocate memory for ping packet.\n" RESET, ping->program_name);
		g_is_running = false;
		return ;
	}

	// Serialize step
	print_bits(ping->packet[0]);
	while (i < sizeof(ping->icmp_packet))
	{
	    ping->packet[i] = *((uint8_t *)buf);
	    buf++;
	    i++;
	}

	while (i < len)
	{
	    ping->packet[i] = payload_data[i - sizeof(ping->icmp_packet)];
	    i++;
	}
	print_bits(ping->packet[0]);
	return ;
}

void	build_ping_packet(t_ping *ping)
{
	AUTO_LOG;

	init_echo_header(ping);
	serialize_icmp_packet(ping);
	if (!ping->packet) return ;
	ping->icmp_packet.checksum = calculate_checksum((uint16_t *)ping->packet, ping->packet_len);
	*(uint16_t *)&ping->packet[2] = ping->icmp_packet.checksum; // this writes both the packet[2] and packet[3]
	*(uint16_t *)&ping->packet[4] = ping->icmp_packet.identifier; // this writes both the packet[4] and packet[5]
	return ;
}

uint16_t	calculate_checksum(void *addr, int count)
{
	AUTO_LOG;

	// Source : https://www.rfc-editor.org/rfc/rfc1071
	/* Compute Internet Checksum for "count" bytes
	*         beginning at location "addr".
	*/
	uint32_t	sum = 0;
	uint16_t	*temp = (uint16_t *)addr;
	while (count > 1)
	{
		/*  This is the inner loop */
		sum += *temp++;
		count -= 2;
	}
	/*  Add left-over byte, if any */
	if (count > 0)
		sum += * (uint8_t *) temp;
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	LOG(GREEN "Calculated checksum: %u" RESET, (uint16_t)(~sum));
	return ((uint16_t)(~sum));
}

void	init_echo_header(t_ping *ping)
{
	AUTO_LOG;

	ping->icmp_packet.type = ICMP_ECHO;
	ping->icmp_packet.code = 0;
	ping->icmp_packet.checksum = 0;
	ping->icmp_packet.identifier = getpid();
	ping->icmp_packet.sequence_number = ping->count;

	LOG(GREEN "Echo header initialized" RESET);
	return ;
}

void    parse_echo_reply(t_ping *ping unused)
{
	AUTO_LOG;
	return ;
}
