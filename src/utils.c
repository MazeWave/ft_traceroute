/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:10:44 by ldalmass          #+#    #+#             */
/*   Updated: 2026/04/29 14:26:59 by ldalmass         ###   ########.fr       */
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

	LOG(YELLOW "signal %d received, stopping ping" RESET, signum);
	printf("stopping...\n");	
	g_is_running = false;
	return;
}

// void	print_end_statistics(t_tr *tr)
// {
// 	AUTO_LOG;

// 	t_replies	*temp = tr->replies;
// 	float		round_trip_avg = 0.0;
// 	float		round_trip_max = 0.0;
// 	float		round_trip_min = INT_MAX;
// 	float		round_trip_ecart_type = 0;
// 	float		round_trip_sum = 0;
// 	float		round_trip_squared_sum = 0;

// 	while (temp)
// 	{
// 		(round_trip_min > temp->elapsed_time_in_ms) ? round_trip_min = temp->elapsed_time_in_ms : round_trip_min;
// 		(round_trip_max < temp->elapsed_time_in_ms) ? round_trip_max = temp->elapsed_time_in_ms : round_trip_max;
// 		round_trip_avg += temp->elapsed_time_in_ms;
// 		round_trip_sum += temp->elapsed_time_in_ms;
// 		round_trip_squared_sum += pow(temp->elapsed_time_in_ms, 2);
// 		temp = temp->next;
// 	}
// 	if (tr->packet_recieved_count > 0) round_trip_avg /= tr->packet_recieved_count;
// 	if (tr->packet_recieved_count > 0) round_trip_ecart_type = sqrt(
// 																		(round_trip_squared_sum / tr->packet_recieved_count)
// 																		- pow(round_trip_sum / tr->packet_recieved_count, 2)
// 																	);
// 	if (isnan(round_trip_ecart_type)) round_trip_ecart_type = 0;

// 	tr->total_time_elapsed = get_time();
// 	printf(YELLOW "\n--- %s ping statistics ---\n" RESET, tr->hostname);
// 	int	packet_loss = (int)((1.0 - ((float)tr->packet_recieved_count / (float)tr->packet_sent_count)) * 100);
// 	if (packet_loss == INT_MAX || packet_loss == INT_MIN) packet_loss = 0;
// 	printf(
// 		YELLOW "%d packets transmitted, %d packets received, %d%% packet loss\n" RESET,
// 		tr->packet_sent_count,
// 		tr->packet_recieved_count,
// 		packet_loss
// 	);
// 	if (tr->packet_recieved_count == 0 && tr->packet_sent_count > 0) return ;
// 	printf(
// 		YELLOW "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n" RESET,
// 		round_trip_min,
// 		round_trip_avg,
// 		round_trip_max,
// 		round_trip_ecart_type
// 	);

// 	return ;
// }

void print_echo_reply(t_tr *tr)
{
	AUTO_LOG;

	if (!tr->replies)
	{
		LOG(RED "%s: No replies received yet.\n" RESET, tr->program_name);
		return;
	}
	t_replies *reply = tr->replies;
	while (reply->next)
		reply = reply->next;

	// if (tr->is_quiet) return ;
	printf(
		GREEN "%u bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n" RESET,
		reply->length,
		tr->ip_str,
		reply->reply.sequence_number,
		reply->reversed_ttl,
		reply->elapsed_time_in_ms);

	return;
}

void print_bits(uint32_t n)
{
	for (int i = 31; i >= 0; i--)
	{
		putchar((n >> i) & 1 ? '1' : '0');
		if (i % 8 == 0)
			putchar(' ');
	}
	putchar('\n');
}

void print_packet_informations(t_tr *tr unused)
{
	AUTO_LOG;

	LOG(CYAN "[HEADER]" RESET);
	LOG(BLUE "Packet sequence: %d" RESET, tr->icmp_packet.sequence_number);
	LOG(BLUE "Type: %d" RESET, tr->icmp_packet.type);
	LOG(BLUE "Code: %d" RESET, tr->icmp_packet.code);
	LOG(CYAN "Checksum: %d" RESET, tr->icmp_packet.checksum);
	LOG(CYAN "Identifier: %d" RESET, tr->icmp_packet.identifier);
	LOG(BLUE "[PAYLOAD]" RESET);
	return;
}

void print_ping_struct(t_tr *tr unused)
{
	AUTO_LOG;
	LOG(BLUE);
	LOG("is_bonus: %d", tr->is_bonus);
	LOG("is_root: %d", tr->is_root);
	// LOG("count: %d", tr->count);
	LOG("hostname: %s", tr->hostname);
	LOG("sockfd: %d", tr->sockfd);
	LOG(RESET);
}