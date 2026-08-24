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
	memcpy(&tr->packet[0], &tr->icmp_packet.type, sizeof(uint8_t));
	memcpy(&tr->packet[1], &tr->icmp_packet.code, sizeof(uint8_t));
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