/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:10:44 by ldalmass          #+#    #+#             */
/*   Updated: 2026/0505/1313 14:3232:5656 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"

struct timeval get_time()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	return (time);
}

bool did_we_timeout(struct timeval start, t_tr *tr)
{
	if (tr->response_timeout_for_each_probe == -1) return (false);
	struct timeval end = get_time();
	int elapsed_time_in_sec = end.tv_sec - start.tv_sec;
	bool did_we_timeout = (elapsed_time_in_sec >= tr->response_timeout_for_each_probe) ? true : false;
	return (did_we_timeout);
}

bool did_we_exceed_in_seconds(struct timeval start, uint32_t seconds)
{
	struct timeval end = get_time();
	int elapsed_time_in_sec = end.tv_sec - start.tv_sec;
	bool did_we_timeout = ((uint32_t)elapsed_time_in_sec >= seconds) ? true : false;
	return (did_we_timeout);
}

void handle_sigint(int signum unused)
{
	AUTO_LOG;

	LOG(YELLOW "signal %d received, stopping traceroute" RESET, signum);
	g_is_running = false;
	return;
}

// void print_echo_reply(t_tr *tr)
// {
// 	AUTO_LOG;

// 	if (!tr->replies)
// 	{
// 		LOG(RED "%s: No replies received yet.\n" RESET, tr->program_name);
// 		return;
// 	}
// 	t_replies *reply = tr->replies;
// 	while (reply->next)
// 		reply = reply->next;

// 	// if (tr->is_quiet) return ;
// 	printf(
// 		GREEN "%u bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n" RESET,
// 		reply->length,
// 		tr->ip_str,
// 		reply->reply.sequence_number,
// 		reply->reversed_ttl,
// 		reply->elapsed_time_in_ms);

// 	return;
// }

// void print_bits(uint32_t n)
// {
// 	for (int i = 31; i >= 0; i--)
// 	{
// 		putchar((n >> i) & 1 ? '1' : '0');
// 		if (i % 8 == 0)
// 			putchar(' ');
// 	}
// 	putchar('\n');
// }

// void print_packet_informations(t_tr *tr unused)
// {
// 	AUTO_LOG;

// 	LOG(CYAN "[HEADER]" RESET);
// 	LOG(BLUE "Packet sequence: %d" RESET, tr->icmp_packet.sequence_number);
// 	LOG(BLUE "Type: %d" RESET, tr->icmp_packet.type);
// 	LOG(BLUE "Code: %d" RESET, tr->icmp_packet.code);
// 	LOG(CYAN "Checksum: %d" RESET, tr->icmp_packet.checksum);
// 	LOG(CYAN "Identifier: %d" RESET, tr->icmp_packet.identifier);
// 	LOG(BLUE "[PAYLOAD]" RESET);
// 	return;
// }

// void print_ping_struct(t_tr *tr unused)
// {
// 	AUTO_LOG;
// 	LOG(BLUE);
// 	LOG("is_bonus: %d", tr->is_bonus);
// 	LOG("is_root: %d", tr->is_root);
// 	// LOG("count: %d", tr->count);
// 	LOG("hostname: %s", tr->hostname);
// 	LOG("sockfd: %d", tr->sockfd);
// 	LOG(RESET);
// }
