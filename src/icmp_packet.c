/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo_header.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/15 15:09:13 by ldalmass          #+#    #+#             */
/*   Updated: 2026/01/16 15:01:48 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

uint16_t	calculate_checksum(uint16_t *addr, int count)
{
	AUTO_LOG;

	// Source : https://www.rfc-editor.org/rfc/rfc1071
	/* Compute Internet Checksum for "count" bytes
	*         beginning at location "addr".
	*/
	uint32_t sum = 0;
	while (count > 1)
	{
		/*  This is the inner loop */
		sum += *addr++;
		count -= 2;
	}
	/*  Add left-over byte, if any */
	if (count > 0)
		sum += * (unsigned char *) addr;
	/*  Fold 32-bit sum to 16 bits */
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return ((uint16_t)(~sum));
}

void	populate_echo_request(t_ping *ping)
{
	AUTO_LOG;
	// LOG(RED "payload_length: %d" RESET, ping->payload_length);
	// LOG(RED "payload_length / 4 + 1: %d" RESET, (ping->payload_length / 4) + 1);
	ping->icmp_packet.payload.data = calloc((ping->payload_length / 4) + 1, sizeof(uint32_t));
	if (!ping->icmp_packet.payload.data)
	{
		LOG(RED "%s: echo_header: Failed to allocate memory for payload" RESET, ping->program_name);
		return;
	}

	for (uint32_t i = 0; i < ping->payload_length; i++)
	{
		for (uint32_t j = 0; j < 4; j++)
		{
			if (i > ping->payload_length)
				break;
			ping->icmp_packet.payload.data[i / 4] |= ping->payload_raw_string[i] << (24 - (j * 8));
			// LOG("letter i: %d, offset j: %d, bitshift: %d", i, j, (24 - (j * 8)));
			// LOG("added char: %c", ping->payload_raw_string[i]);
			i++;
		}
		i--;
	}
	// Calculate the checksum
	calculate_checksum((uint16_t *)(&ping->icmp_packet), sizeof(struct s_echo_header) + ping->payload_length);


	// Print all the chars in the payload
	// ping->echo_request.checksum = calculate_checksum(ping->echo_request);
	for (uint8_t i = 0; i < ping->payload_length / 4 + 1; i++)
	{
		uint32_t	value = ping->icmp_packet.payload.data[i];
		LOG(MAGENTA "1th char: %c" RESET, (char)(value >> 24));
		LOG(MAGENTA "2rd char: %c" RESET, (char)(value >> 16));
		LOG(MAGENTA "3nd char: %c" RESET, (char)(value >> 8));
		LOG(MAGENTA "4st char: %c" RESET, (char)(value));
		LOG("");
	}
}

t_echo_header	init_echo_header(void)
{
	AUTO_LOG;

	t_echo_header	echo_header;
	echo_header.type = ICMP_ECHO;
	echo_header.code = 0;
	echo_header.checksum = 0;
	echo_header.identifier = 0;
	echo_header.sequence_number = 0;

	return (echo_header);
}

void    parse_echo_reply(t_ping *ping unused)
{
	AUTO_LOG;
	return;
}
