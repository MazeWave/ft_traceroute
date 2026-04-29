/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:26:14 by maze              #+#    #+#             */
/*   Updated: 2026/04/29 13:16:47 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_traceroute.h"

volatile bool g_is_running = true;

static void ping_loop(t_tr *tr)
{
	AUTO_LOG;

	// printf(
	// 	BLUE "PING %s (%s): %u data bytes" RESET,
	// 	tr->hostname,
	// 	tr->ip_str,
	// 	tr->payload_length
	// );

	struct timeval timeout_start = get_time();

	// Main ping loop
	while (g_is_running)
	{
		if (did_we_timeout(timeout_start, tr)) g_is_running = false;
		if (tr->count == 0) break;
		if (tr->count != -1) tr->count--;
		if (!g_is_running) break;

		// Build the ping packet
		build_ping_packet(tr);

		// Send the ping
		struct timeval start = get_time();
		send_packet(tr);

		// Listen to the echo reply
		float elapsed_time_in_seconds = deserialize_icmp_packet(tr, start);

		// Recieve the echo icmp packet and display timings
		print_echo_reply(tr);

		// Account for interval
		float remaining = tr->interval - elapsed_time_in_seconds;
		if (tr->timeout != -1)
		{
			struct timeval deadline = get_time();
			// remaining = tr->timeout - (deadline.tv_sec - timeout_start.tv_sec);
			float deadline_time = tr->timeout - (deadline.tv_sec - timeout_start.tv_sec);
			if (deadline_time < remaining)
				remaining = deadline_time;
		}
		if (remaining > 0)
		{
			LOG(DEBUG RED "remaining : %f seconds" RESET, remaining);
			struct timespec ts;
			ts.tv_sec = remaining;
			ts.tv_nsec = (remaining - ts.tv_sec) * 1000000000;
			nanosleep(&ts, NULL);
		}
		// tr->packet_sent_count++;
	}

	// Linger option
	// struct timeval	linger_start = get_time();
	// if (tr->linger != -1)
	// {
	// 	while (
	// 		   !did_we_exceed_in_seconds(linger_start, tr->linger)
	// 		&& !did_we_timeout(timeout_start, ping)
	// 		&& g_is_running
	// 	)
	// 		deserialize_icmp_packet(ping, linger_start);
	// }
	// print_end_statistics(ping);
	// return;
}

static void free_ping(t_tr *tr unused)
{
	AUTO_LOG;

	// Free the addrinfo linked list
	struct addrinfo *addr = tr->addr_info;
	while (addr)
	{
		struct addrinfo *next = addr->ai_next;
		free(addr);
		addr = next;
	}

	// Free the echo replies linked list
	t_replies *echo_reply = tr->replies;
	while (echo_reply)
	{
		t_replies *next = echo_reply->next;
		free(echo_reply);
		echo_reply = next;
	}

	// Free the ip_str
	if (tr->ip_str) free(tr->ip_str);
	return;
}

int main(int argc, char **argv unused)
{
	AUTO_LOG;

	t_tr tracerutu;
	t_tr *tr = &tracerutu;
	init_ping_struct(tr, argv);

	if (argc < 2)
		return (help(tr), EXIT_FAILURE);

	signal(SIGINT, &handle_sigint);
	signal(SIGQUIT, &handle_sigint);

	if (parse_args(argc, argv, tr) == EXIT_FAILURE) return (tr->exit_status);
	if (create_icmp_socket(tr) == EXIT_FAILURE) return (EXIT_FAILURE);
	if (resolve_hostname(tr) == EXIT_FAILURE) return (EXIT_FAILURE);

	ping_loop(tr);
	close(tr->sockfd);
	free_ping(tr);
	return (0);
}
