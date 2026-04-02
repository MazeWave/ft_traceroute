/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:26:14 by maze              #+#    #+#             */
/*   Updated: 2026/03/25 18:37:19 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

volatile bool	g_is_running = true;

static void	ping_loop(t_ping *ping)
{
	AUTO_LOG;

	printf(
		BLUE "PING %s (%s): %u data bytes" RESET,
		ping->hostname,
		ping->ip_str,
		ping->payload_length
	);
	
	struct timeval	timeout_start = get_time();

	if (ping->is_verbose) printf(BLUE ", id 0x%x = %d\n" RESET, getpid() & 0xffff, getpid() & 0xffff);
	else printf("\n");

	// Flooding option
	while (ping->is_flooding && g_is_running)
	{
		if (did_we_timeout(timeout_start, ping)) break;
		if (ping->count == 0) break;
		struct timeval	start = get_time();
		build_ping_packet(ping);
		printf(".");
		fflush(stdout);
		send_ping(ping);
		ping->packet_sent_count++;
		if (ping->count != -1) ping->count--;
		float ret = 0.0;
		while ((ret = deserialize_icmp_packet(ping, start)) > 0.0)
		{
			if (did_we_timeout(timeout_start, ping)) break;
			printf("\b");
			fflush(stdout);
		}
	}

	// Preload option
	if (ping->preload_count > 0)
	{
		for (int i = 0; i < ping->preload_count; i++)
		{
			if (did_we_timeout(timeout_start, ping)) g_is_running = false;
			if (!g_is_running) break;
			build_ping_packet(ping);
			send_ping(ping);
			ping->packet_sent_count++;
			if (ping->count > 0) ping->count--;
		}
		ping->preload_count = 0;
		struct timeval	start = get_time();
		while (deserialize_icmp_packet(ping, start) > 0.0)
			print_echo_reply(ping);
	}

	// Main ping loop
	while (g_is_running)
	{
		if (did_we_timeout(timeout_start, ping)) g_is_running = false;
		if (ping->count == 0) break;
		if (ping->count != -1) ping->count--;
		if (!g_is_running) break;

		// Build the ping packet
		build_ping_packet(ping);

		// Send the ping
		struct timeval	start = get_time();
		send_ping(ping);

		// Listen to the echo replay
		float	elapsed_time_in_seconds = deserialize_icmp_packet(ping, start);

		// Recieve the echo icmp packet and display timings
		print_echo_reply(ping);

		// Account for interval
		float	remaining = ping->interval - elapsed_time_in_seconds;
		if (ping->timeout != -1)
		{
			struct timeval deadline = get_time();
			// remaining = ping->timeout - (deadline.tv_sec - timeout_start.tv_sec);
			float deadline_time = ping->timeout - (deadline.tv_sec - timeout_start.tv_sec);
			if (deadline_time < remaining)
				remaining = deadline_time;
		}
		if (remaining > 0)
		{
			LOG(DEBUG RED "remaining : %f seconds" RESET, remaining);
			struct timespec	ts;
			ts.tv_sec = remaining;
			ts.tv_nsec = (remaining - ts.tv_sec) * 1000000000;
			nanosleep(&ts, NULL);
		}
		ping->packet_sent_count++;
	}

	// Linger option
	struct timeval	linger_start = get_time();
	if (ping->linger != -1)
	{
		while (
			   !did_we_exceed_in_seconds(linger_start, ping->linger)
			&& !did_we_timeout(timeout_start, ping)
			&& g_is_running
		)
			deserialize_icmp_packet(ping, linger_start);
	}
	print_end_statistics(ping);
	return;
}

static void	free_ping(t_ping *ping unused)
{
	AUTO_LOG;

	// Free the addrinfo linked list
	struct addrinfo	*addr = ping->addr_info;
	while (addr)
	{
		struct addrinfo	*next = addr->ai_next;
		free(addr);
		addr = next;
	}

	// Free the echo replies linked list
	t_replies	*echo_reply = ping->replies;
	while (echo_reply)
	{
		t_replies	*next = echo_reply->next;
		free(echo_reply);
		echo_reply = next;
	}

	// Free the ip_str
	if (ping->ip_str) free(ping->ip_str);
	return ;
}

int	main(int argc, char **argv unused)
{
	AUTO_LOG;

	t_ping	pingu;
	t_ping	*ping = &pingu;
	init_ping_struct(ping, argv);

	if (argc < 2)
		return (help(ping), EXIT_FAILURE);

	signal(SIGINT, &handle_sigint);
	signal(SIGQUIT, &handle_sigint);

	if (parse_args(argc, argv, ping) == EXIT_FAILURE) return (ping->exit_status);
	if (create_icmp_socket(ping) == EXIT_FAILURE) return (EXIT_FAILURE);
	if (resolve_hostname(ping) == EXIT_FAILURE) return (EXIT_FAILURE);

	ping_loop(ping);
	close(ping->sockfd);
	free_ping(ping);
	return (0);
}
