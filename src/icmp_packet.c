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

void	build_ping_packet(t_ping *ping)
{
	AUTO_LOG;

	init_icmp_header(ping);
	serialize_icmp_packet(ping);
	if (!ping->packet) return ;
	ping->icmp_packet.checksum = calculate_checksum((uint16_t *)ping->packet, ping->packet_len);
	*(uint16_t *)&ping->packet[2] = ping->icmp_packet.checksum;		// this writes both the packet[2] and packet[3]
	// *(uint16_t *)&ping->packet[4] = ping->icmp_packet.identifier;	// this writes both the packet[4] and packet[5]
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

void	init_icmp_header(t_ping *ping)
{
	AUTO_LOG;
	
	static	int	sequence_number = 0;

	ping->icmp_packet.type = ICMP_ECHO;
	ping->icmp_packet.code = 0;
	ping->icmp_packet.checksum = 0;
	ping->icmp_packet.identifier = getpid() & 0xffff;
	ping->icmp_packet.sequence_number = sequence_number++;

	LOG(GREEN "Echo header initialized" RESET);
	return ;
}

void    parse_echo_reply(t_ping *ping unused)
{
	AUTO_LOG;
	return ;
}
