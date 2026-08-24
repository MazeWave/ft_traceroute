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
	tr->udp_packet.checksum = calculate_checksum((uint16_t *)tr->packet, tr->packet_len);
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
