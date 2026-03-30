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
#include <limits.h>

volatile bool	g_is_running = true;

static void	print_end_statistics(t_ping *ping)
{
	AUTO_LOG;

	t_replies	*temp = ping->replies;
	float		round_trip_avg = 0.0;
	float		round_trip_max = 0.0;
	float		round_trip_min = INT_MAX;
	float		round_trip_ecart_type = 0;
	float		round_trip_sum = 0;
	float		round_trip_squared_sum = 0;


	while (temp)
	{
		(round_trip_min > temp->elapsed_time_in_ms) ? round_trip_min = temp->elapsed_time_in_ms : round_trip_min;
		(round_trip_max < temp->elapsed_time_in_ms) ? round_trip_max = temp->elapsed_time_in_ms : round_trip_max;
		round_trip_avg += temp->elapsed_time_in_ms;
		round_trip_sum += temp->elapsed_time_in_ms;
		round_trip_squared_sum += pow(temp->elapsed_time_in_ms, 2);
		temp = temp->next;
	}
	if (ping->packet_recieved_count > 0) round_trip_avg /= ping->packet_recieved_count;
	if (ping->packet_recieved_count > 0) round_trip_ecart_type = sqrt(
																		(round_trip_squared_sum / ping->packet_recieved_count)
																		- pow(round_trip_sum / ping->packet_recieved_count, 2)
																	);

	struct timeval	total_time = ping->total_time_elapsed;
	gettimeofday(&ping->total_time_elapsed, NULL);
	uint64_t	total_time_in_ms =	(ping->total_time_elapsed.tv_sec * 1000.0 + ping->total_time_elapsed.tv_usec / 1000.0)
									- (total_time.tv_sec * 1000.0 + total_time.tv_usec / 1000.0);
	printf(YELLOW "\n--- %s ping statistics ---\n" RESET, ping->hostname);
	printf(
		YELLOW "%d packets transmitted, %d packets recieved, %.3f%% packet loss, time %lums\n" RESET,
		ping->packet_sent_count,
		ping->packet_recieved_count,
		1.0 - ((float)ping->packet_recieved_count / (float)ping->packet_sent_count),
		total_time_in_ms
	);
	printf(
		YELLOW "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n" RESET,
		round_trip_min,
		round_trip_avg,
		round_trip_max,
		round_trip_ecart_type
	);

	return ;
}

static void	handle_sigint(int signum unused)
{
	AUTO_LOG;

	LOG(YELLOW "signal %d received, stopping ping" RESET, signum);
	g_is_running = false;
	return;
}

static void	ping_loop(t_ping *ping)
{
	AUTO_LOG;

	signal(SIGINT, &handle_sigint);
	signal(SIGQUIT, &handle_sigint);

	printf(
		BLUE "PING %s (%s) %u data bytes.\n" RESET,
		ping->hostname,
		ping->ip_str,
		ping->payload_length
	);

	// Flooding option
	while (ping->is_flooding && g_is_running)
	{
		if (ping->count == 0) break;
		struct timeval	start;
		gettimeofday(&start, NULL);
		build_ping_packet(ping);
		printf(".");
		fflush(stdout);
		send_ping(ping);
		if (deserialize_icmp_packet(ping, start) != -1.0)
		{
			printf("\b");
			fflush(stdout);
		}
		ping->packet_sent_count++;
		if (ping->count != -1) ping->count--;
	}

	// Preload option
	for (int i = 0; i < ping->preload_count; i++)
	{
		struct timeval	start;
		gettimeofday(&start, NULL);
		build_ping_packet(ping);
		send_ping(ping);
		deserialize_icmp_packet(ping, start);
		ping->packet_sent_count++;
	}

	// Main ping loop
	while (g_is_running)
	{
		// Account for count
		if (ping->count == 0) break;
		if (ping->count != -1) ping->count--;
		LOG(YELLOW "pinging... count: %d" RESET, ping->count + 1);

		// Build the ping packet
		build_ping_packet(ping);
		if (!g_is_running) break;

		// Send the ping
		struct timeval	start;
		gettimeofday(&start, NULL);
		send_ping(ping);

		// Listen to the echo replay
		float	elapsed_time_in_seconds = deserialize_icmp_packet(ping, start);

		// Display the ping result
		print_echo_reply(ping);

		// Account for interval
		// gettimeofday(&end, NULL);
		// uint64_t	elapsed_usec = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
		float	remaining = ping->interval - elapsed_time_in_seconds;
		if (remaining > 0)
		{
			LOG(DEBUG RED "remaining : %f seconds" RESET, remaining);
			struct timespec	ts =
			{
				.tv_sec = remaining,
				.tv_nsec = remaining * 1000000000
			};
			nanosleep(&ts, NULL);
		}
		ping->packet_sent_count++;
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

	if (argc < 2)
		return (help(argv[0]), EXIT_FAILURE);

	t_ping	pingu;
	t_ping	*ping = &pingu;

	init_ping_struct(ping, argv);
	if (parse_args(argc, argv, ping) == EXIT_FAILURE) return (EXIT_FAILURE);
	if (create_icmp_socket(ping) == EXIT_FAILURE) return (EXIT_FAILURE);
	if (resolve_hostname(ping) == EXIT_FAILURE) return (EXIT_FAILURE);

	ping_loop(ping);
	close(ping->sockfd);
	free_ping(ping);
	return (0);
}
