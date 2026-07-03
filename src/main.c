/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:26:14 by maze              #+#    #+#             */
/*   Updated: 2026/05/15 15:03:01 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"

volatile bool g_is_running = true;

static void print_hop_count_formatted(uint8_t n)
{
	if (n < 10) printf(CYAN"  %d   "RESET, n);
	else printf(CYAN" %d   "RESET, n);
	return ;
}

static bool did_we_traceroute_to_target(t_tr *tr)
{
	AUTO_LOG;
	t_replies *temp = tr->replies;

	// Get last response node
	if (!temp) return false;
	while(temp->next) temp = temp->next;

	LOG(DEBUG MAGENTA "target string = %s" RESET, tr->hostname);
	LOG(DEBUG MAGENTA "reversed dns string = %s" RESET, temp->reversed_dns_str);
	LOG(DEBUG MAGENTA "target IP = %u" RESET, tr->ip);
	LOG(DEBUG MAGENTA "reversed IP = %u" RESET, temp->reversed_ip);
	LOG(DEBUG MAGENTA "icmp reply type : %d" RESET, temp->reply.type);
	LOG(DEBUG MAGENTA "reply type == 0 -> ICMP_ECHOREPLY" RESET, temp->reply.type);
	LOG(DEBUG MAGENTA "reply type == 11 -> ICMP_TIME_EXCEEDED" RESET, temp->reply.type);
	LOG(DEBUG MAGENTA "reply type == 3 -> ICMP_DEST_UNREACH" RESET, temp->reply.type);

	// Actual check
	if (temp->reply.type == ICMP_ECHOREPLY && tr->ip == temp->reversed_ip) g_is_running = false;
	return (temp->reply.type == ICMP_ECHOREPLY && tr->ip == temp->reversed_ip);
}

static char *get_last_ip_str_returned(t_tr *tr)
{
	t_replies	*temp = tr->replies;

	if (!temp) return (NULL);
	while (temp->next) temp = temp->next;
	return (strdup(temp->reversed_ip_str));
}

static char *get_last_dns_str_returned(t_tr *tr)
{
	t_replies	*temp = tr->replies;

	if (!temp) return (NULL);
	while (temp->next) temp = temp->next;
	return (strdup(temp->reversed_dns_str));
}

static void traceroute_loop(t_tr *tr)
{
	AUTO_LOG;

	LOG(MAGENTA "TTL = %d" RESET, tr->ttl);
	LOG(MAGENTA "TTL + offset (%d) = %d" RESET, tr->offset_hop, tr->ttl + tr->offset_hop);
	LOG(MAGENTA "max hops = %d" RESET, tr->max_hops);
	LOG(MAGENTA "probes_per_hop = %d" RESET, tr->probes_per_hop);
	LOG(MAGENTA "offset_hop = %d" RESET, tr->offset_hop);
	LOG(MAGENTA "response_time = %d" RESET, tr->response_timeout_for_each_probe);
	LOG(MAGENTA "port = %d" RESET, tr->tos);

	printf(GREEN "traceroute to %s (%s), %d hops max\n" RESET, tr->hostname, tr->ip_str, tr->max_hops);
	uint8_t	hop_count = 0;
	while (g_is_running)
	{
		print_hop_count_formatted(hop_count + 1);

		uint8_t	probe_count = 0;
		while (probe_count < tr->probes_per_hop && g_is_running)
		{
			// Increment the probe count (default 3 probes per hops)
			probe_count++;
			did_we_traceroute_to_target(tr); // Sets g_is_running to false if we are at the target

			// Build the probe
			build_traceroute_packet(tr);

			// Send the proble
			send_packet(tr);

			// Listen for the reply
			struct timeval start = get_time();
			float time_taken = deserialize_icmp_packet(tr, start);
			LOG(DEBUG CYAN "is condition true : %d" RESET, (get_time().tv_sec - start.tv_sec) > tr->response_timeout_for_each_probe);

			// This while functions like a linger,
			// instead of blocking the program for a fixed amount of time,
			// it polls for a response until the response timeout is reached,
			// allowing to quit the program if a signal is received within 100ms
			while (
				   g_is_running
				&& (get_time().tv_sec - start.tv_sec) < tr->response_timeout_for_each_probe
				&& (time_taken == -1.0)
			)
			{
				// This poll each 100ms to wait for a response
				time_taken = deserialize_icmp_packet(tr, start);
				LOG(DEBUG MAGENTA "POLL" RESET);
			}

			// Print informations
			char *last_reversed_ip_str = get_last_ip_str_returned(tr);
			char *last_reversed_dns_str = get_last_dns_str_returned(tr);
			if (probe_count == 1 && last_reversed_ip_str && time_taken != -1.0) printf(MAGENTA "%s  " RESET, last_reversed_ip_str);
			if (probe_count == 1 && last_reversed_dns_str && tr->do_reverse_dns && time_taken != -1.0)
			{
				if (last_reversed_dns_str[0] == '\0') printf (YELLOW "(%s)  " RESET, last_reversed_ip_str);
				else printf(YELLOW "(%s)  " RESET, last_reversed_dns_str);
			}
			if (last_reversed_ip_str) free(last_reversed_ip_str);
			if (last_reversed_dns_str) free(last_reversed_dns_str);
			if (time_taken == -1.0) printf(RED "*  " RESET);
			else printf("%.3""fms  ", time_taken);
			// fflush(stdout);
			if (probe_count == tr->probes_per_hop) printf("\n"); // New line after the last probe result
		}

		did_we_traceroute_to_target(tr); // Sets g_is_running to false if we are at the target
		
		// Increment the ttl and hop count
		tr->ttl++;
		uint32_t	final_ttl = tr->ttl + tr->offset_hop;
		setsockopt(tr->sockfd, IPPROTO_IP, IP_TTL, &final_ttl, sizeof(final_ttl));
		hop_count++;
		// Check if we surpassed the max_hop count
		if (tr->ttl > tr->max_hops) g_is_running = false;
	}
	return ;
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
		if (echo_reply->reversed_ip_str) free(echo_reply->reversed_ip_str);
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
	close(tr->sockfd);
	free_traceroute(tr);
	return (0);
}
