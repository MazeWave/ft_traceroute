/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   icmp_packet.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:09:13 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/25 14:02:59 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"
#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <stdint.h>

void	serialize_icmp_packet(t_ping *ping, size_t packet_len)
{
	size_t		i = 0;
	size_t		len = sizeof(ping->icmp_packet.header) + ping->payload_length;
	uint8_t		*buf = (uint8_t *)&ping->icmp_packet.header;
	uint8_t		*payload_data = (uint8_t *)&ping->payload_raw_string;

	// uint8_t		*packet = malloc(packet_len);
	uint8_t		*packet = calloc(packet_len, sizeof(uint8_t));
	if (!packet)
	{
		printf(RED "%s: malloc: Failed to allocate memory for ping packet.\n" RESET, ping->program_name);
		g_is_running = false;
		return ;
	}

	// Serialize step
	while (i < sizeof(ping->icmp_packet.header))
	{
	    packet[i] = *((uint8_t *)buf);
	    buf++;
	    i++;
	}
	while (i < len)
	{
	    packet[i] = payload_data[i - sizeof(ping->icmp_packet.header)];
	    i++;
	}
	ping->packet = packet;
	return ;
}

void	build_ping_packet(t_ping *ping)
{
	AUTO_LOG;

	init_echo_header(ping);

	serialize_icmp_packet(ping, ping->packet_len);
	if (!ping->packet) return ;
	// Checksum step
	calculate_checksum((uint16_t *)ping->packet, ping->packet_len / 2);


	// sendto(socketfd, packet, len, flags, dest_addr, addr_len);
	send_ping(ping);
	// Barbarian checksum calculation, we take the entire struct and cast it as uint16_t so we can checksum it.
	// Other function are more self explanatory
	ping->icmp_packet.header.checksum = calculate_checksum((uint16_t *)(&ping->icmp_packet), sizeof(struct s_echo_header) + ping->payload_length);
	print_ping_struct(ping);
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

// void	add_payload_to_packet(t_ping *ping)
// {
// 	AUTO_LOG;

// 	// LOG(RED "payload_length: %d" RESET, ping->payload_length);
// 	// LOG(RED "payload_length / 4 + 1: %d" RESET, (ping->payload_length / 4) + 1);

// 	// If there is no payload, skip
// 	if (
// 		!ping->payload_raw_string ||
// 		!ping->icmp_packet.payload.data ||
// 		ping->payload_length == 0 ||
// 		ping->payload_length > PING_MAX_DATA_LEN
// 	)
// 	{
// 		LOG(YELLOW "No payload provided" RESET);
// 		return ;
// 	}
// 	// If we have already written the payload, skip
// 	if (ping->icmp_packet.payload.data[0] != 0) return;

// 	// Convert the raw string payload into the uint32_t array
// 	for (uint32_t i = 0; i < ping->payload_length; i++)
// 	{
// 		for (uint32_t j = 0; j < 4; j++)
// 		{
// 			if (i > ping->payload_length)
// 				break;
// 			ping->icmp_packet.payload.data[i / 4] |= ping->payload_raw_string[i] << (24 - (j * 8));
// 			// LOG("letter i: %d, offset j: %d, bitshift: %d", i, j, (24 - (j * 8)));
// 			// LOG("added char: %c", ping->payload_raw_string[i]);
// 			i++;
// 		}
// 		i--;
// 	}
// 	return ;
// }

// void	read_payload_data_in_packet(t_ping *ping)
// {
// 	AUTO_LOG;

// 	if (!ping->payload_raw_string)
// 	{
// 		LOG(YELLOW "No payload to read from" RESET);
// 		return;
// 	}

// 	// Print all the chars in the payload
// 	// ping->echo_request.checksum = calculate_checksum(ping->echo_request);
// 	for (uint8_t i = 0; i < ping->payload_length / 4 + 1; i++)
// 	{
// 		uint32_t	value = ping->icmp_packet.payload.data[i];
// 		LOG(MAGENTA "1th char: %c" RESET, (char)(value >> 24));
// 		LOG(MAGENTA "2rd char: %c" RESET, (char)(value >> 16));
// 		LOG(MAGENTA "3nd char: %c" RESET, (char)(value >> 8));
// 		LOG(MAGENTA "4st char: %c" RESET, (char)(value));
// 		LOG("");
// 	}
// 	return ;
// }

void	init_echo_header(t_ping *ping)
{
	AUTO_LOG;

	ping->icmp_packet.header.type = ICMP_ECHO;
	ping->icmp_packet.header.code = 0;
	ping->icmp_packet.header.checksum = 0;
	ping->icmp_packet.header.identifier = 0;
	ping->icmp_packet.header.sequence_number = ping->count;

	// if (!ping->icmp_packet.payload.data) ping->icmp_packet.payload.data = calloc((ping->payload_length / 4) + 1, sizeof(uint32_t));
	// if (!ping->icmp_packet.payload.data) LOG(RED "%s: icmp_packet: Failed to allocate memory for payload" RESET, ping->program_name);
	// ping->icmp_packet.payload.length = (ping->payload_length / 4) + 1;

	LOG(GREEN "Echo header initialized" RESET);
	return ;
}

void    parse_echo_reply(t_ping *ping unused)
{
	AUTO_LOG;
	return ;
}
