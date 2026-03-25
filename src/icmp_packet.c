/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp_packet.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:09:13 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/25 14:53:31 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"
#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <stdint.h>

void	serialize_icmp_packet(t_ping *ping)
{
	AUTO_LOG;

	size_t		i = 0;
	size_t		len = sizeof(ping->icmp_packet.header) + ping->payload_length;
	uint8_t		*buf = (uint8_t *)&ping->icmp_packet.header;
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
	while (i < sizeof(ping->icmp_packet.header))
	{
	    ping->packet[i] = *((uint8_t *)buf);
	    buf++;
	    i++;
	}
	
	while (i < len)
	{
	    ping->packet[i] = payload_data[i - sizeof(ping->icmp_packet.header)];
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
	ping->icmp_packet.header.checksum = calculate_checksum((uint16_t *)ping->packet, ping->packet_len);
	*(uint16_t *)&ping->packet[2] = ping->icmp_packet.header.checksum; // this writes both the packet[2] and packet[3]
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

	ping->icmp_packet.header.type = ICMP_ECHO;
	ping->icmp_packet.header.code = 0;
	ping->icmp_packet.header.checksum = 0;
	ping->icmp_packet.header.identifier = 0;
	ping->icmp_packet.header.sequence_number = ping->count;

	LOG(GREEN "Echo header initialized" RESET);
	return ;
}

void    parse_echo_reply(t_ping *ping unused)
{
	AUTO_LOG;
	return ;
}
