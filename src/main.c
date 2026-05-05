/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:26:14 by maze              #+#    #+#             */
/*   Updated: 2026/04/29 16:35:56 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"
#include <stdint.h>
#include <unistd.h>

volatile bool g_is_running = true;

static void print_hop_count_formatted(uint8_t n)
{
	if (n < 10) printf("  %d   ", n);
	else printf(" %d   ", n);
	return ;
}

static void did_we_traceroute_to_target(t_tr *tr)
{
	AUTO_LOG;
	t_replies *temp = tr->replies;

	if (!temp)
		return ;
	// Get last response node
	while(temp->next) temp = temp->next;
	LOG(DEBUG MAGENTA "target string = %s" RESET, tr->hostname);
	LOG(DEBUG MAGENTA "reversed dn string = %s" RESET, temp->reversed_dns_str);
	// if (tr->ip, temp->) g_is_running = false;
	if (strstr(tr->hostname, temp->reversed_dns_str) != NULL) g_is_running = false;
	return ;
}

static void traceroute_loop(t_tr *tr unused)
{
	AUTO_LOG;

	LOG(MAGENTA "TTL = %d" RESET, tr->ttl);
	LOG(MAGENTA "TTL + offset (%d) = %d" RESET, tr->offset_hop, tr->ttl + tr->offset_hop);
	LOG(MAGENTA "max hops = %d" RESET, tr->max_hops);
	LOG(MAGENTA "probes_per_hop = %d" RESET, tr->probes_per_hop);
	LOG(MAGENTA "offset_hop = %d" RESET, tr->offset_hop);
	LOG(MAGENTA "response_time = %d" RESET, tr->response_time);
	LOG(MAGENTA "port = %d" RESET, tr->port);

	printf(GREEN "traceroute to %s (%s), %d hops max\n" RESET, tr->hostname, tr->ip_str, tr->max_hops);
	uint8_t	hop_count = 0;
	while (g_is_running)
	{
		did_we_traceroute_to_target(tr);
		print_hop_count_formatted(hop_count);
		uint8_t	probe_count = 0;
		while (g_is_running && probe_count < tr->probes_per_hop)
		{
			probe_count++;
			did_we_traceroute_to_target(tr);
			build_ping_packet(tr);
			send_packet(tr);
			struct timeval start = get_time();
			float time_taken = deserialize_icmp_packet(tr, start);
			if (time_taken == -1.0)
			{
				sleep(tr->response_time);
				printf("*  ");
			}
			else printf("%f", time_taken);
			if (probe_count < tr->probes_per_hop) printf("\n");
		}
	}
	return ;
}

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
		// if (did_we_timeout(timeout_start, tr)) g_is_running = false;
		if (tr->count > tr->max_hops) g_is_running = false;
		// if (tr->count == 0) break;
		// if (tr->count != -1) tr->count--;
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
		if (tr->response_time != -1)
		{
			struct timeval deadline = get_time();
			// remaining = tr->timeout - (deadline.tv_sec - timeout_start.tv_sec);
			float deadline_time = tr->response_time - (deadline.tv_sec - timeout_start.tv_sec);
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
		tr->count++;
		LOG(DEBUG "count = %d" RESET, tr->count);
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
	return;
}

static void free_traceroute(t_tr *tr unused)
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
	init_traceroute_struct(tr, argv);

	if (argc < 2) return (help(tr), EXIT_FAILURE);

	signal(SIGINT, &handle_sigint);
	signal(SIGQUIT, &handle_sigint);

	if (parse_args(argc, argv, tr) == EXIT_FAILURE) return (tr->exit_status);
	if (create_icmp_socket(tr) == EXIT_FAILURE) return (EXIT_FAILURE);
	if (resolve_hostname(tr) == EXIT_FAILURE) return (EXIT_FAILURE);

	traceroute_loop(tr);
	if (0) ping_loop(tr);
	close(tr->sockfd);
	free_traceroute(tr);
	return (0);
}
