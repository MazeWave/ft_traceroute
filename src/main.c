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

	// Preload option
	for (int i = 0; i < ping->preload_count; i++)
	{
		build_ping_packet(ping);
		send_ping(ping);
	}

	// Main ping loop
	while (g_is_running)
	{
		// Account for count
		if (ping->count == 0 || ping->ttl == 0) break;
		if (ping->count != -1) ping->count--;
		LOG(YELLOW "pinging... count: %d" RESET, ping->count + 1);

		// Build the ping packet
		build_ping_packet(ping);
		if (!g_is_running) break;

		// Send the ping
		struct timeval	start, end;
		gettimeofday(&start, NULL);
		send_ping(ping);
		
		// Listen to the echo replay
		deserialize_icmp_packet(ping);

		// Account for interval
		gettimeofday(&end, NULL);
		uint64_t	elapsed_usec = (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);                                                                                                                                 
		uint64_t	remaining = (ping->interval * 1000000.0) - elapsed_usec;
		if (remaining > 0)
		{
			struct timespec	ts =
			{
				.tv_sec = remaining / 1000000,
				.tv_nsec = (remaining % 1000000) * 1000
			};
			nanosleep(&ts, NULL);
		}
	}
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
