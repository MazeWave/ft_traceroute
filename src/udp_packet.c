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

void	build_udp_packet(t_tr *tr)
{
	AUTO_LOG;

	tr->packet = calloc(tr->packet_len, sizeof(uint8_t));
	if (!tr->packet)
	{
		LOG(RED "%s: calloc: Failed to allocate memory for udp packet." RESET);
		g_is_running = false;
		return;
	}

	return ;
}
