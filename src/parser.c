/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 14:35:54 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/25 1717:2020:4545 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

void	print_echo_reply(t_ping *ping)
{
	AUTO_LOG;

	t_replies	*reply = ping->replies;
	while (reply->next)
		reply = reply->next;

	switch (ping->is_root)
	{
	case true:
		printf(
			GREEN "%u bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n" RESET,
			reply->length,
			ping->ip_str,
			reply->reply.sequence_number,
			reply->ttl,
			reply->elapsed_time_in_ms
		);
		return ;
	case false:
		printf(
			GREEN "%u bytes from %s: icmp_seq=%d time=%.3f ms\n" RESET,
			reply->length,
			ping->ip_str,
			reply->reply.sequence_number,
			reply->elapsed_time_in_ms
		);
		return ;
	}
}

void	print_bits(uint32_t n)
{
	for (int i = 31; i >= 0; i--)
	{
		putchar((n >> i) & 1 ? '1' : '0');
		if (i % 8 == 0)
			putchar(' ');
	}
	putchar('\n');
}

void	print_packet_informations(t_ping *ping unused)
{
	AUTO_LOG;

	LOG(CYAN "[HEADER]" RESET);
	LOG(BLUE "Packet sequence: %d" RESET, ping->icmp_packet.sequence_number);
	LOG(BLUE "Type: %d" RESET, ping->icmp_packet.type);
	LOG(BLUE "Code: %d" RESET, ping->icmp_packet.code);
	LOG(CYAN "Checksum: %d" RESET, ping->icmp_packet.checksum);
	LOG(CYAN "Identifier: %d" RESET, ping->icmp_packet.identifier);
	LOG(BLUE "[PAYLOAD]" RESET);
	return ;
}

void    print_ping_struct(t_ping *ping unused)
{
	AUTO_LOG;
	LOG(BLUE);
	LOG("is_bonus: %d", ping->is_bonus);
	LOG("is_root: %d", ping->is_root);
	LOG("count: %d", ping->count);
	LOG("hostname: %s", ping->hostname);
	LOG("sockfd: %d", ping->sockfd);
	LOG(RESET);
}

void	get_sockaddr(struct sockaddr_in *ai_addr, t_ping *ping)
{
	AUTO_LOG;

	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ai_addr->sin_addr, ip_str, INET_ADDRSTRLEN);

	// set the ip
	ping->ip = ai_addr->sin_addr.s_addr;
	if (ping->ip_str) free(ping->ip_str);
	ping->ip_str = strndup(ip_str, INET_ADDRSTRLEN);
	if (!ping->ip_str)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for ip string.\n" RESET, ping->program_name);
		ping->ip_str = NULL;
		return ;
	}
	LOG(GREEN "ip as int: %d" BLUE, ping->ip);
	LOG(GREEN "ip as string: %s" BLUE, ip_str);
	return ;
}

void	find_the_ip(t_ping *ping)
{
	AUTO_LOG;

	// return;
	struct addrinfo	*temp = ping->addr_info;
	while (temp)
	{
		LOG(BLUE);
		LOG("--------------------------------");
		LOG("ai_canonname: %s", temp->ai_canonname);
		LOG("ai_family: %d", temp->ai_family);
		LOG("ai_socktype: %d", temp->ai_socktype);
		LOG("ai_protocol: %d", temp->ai_protocol);
		LOG("ai_addrlen: %d", temp->ai_addrlen);
		LOG("ai_addr: %p", temp->ai_addr);
		get_sockaddr((struct sockaddr_in *)temp->ai_addr, ping);
		LOG("ai_canonname: %s", temp->ai_canonname);
		LOG(RESET);
		temp = temp->ai_next;
	}
}

void	help(char *elf_name)
{
	AUTO_LOG;
	printf(GREEN "Usage: %s <hostname> [options]\n" RESET, elf_name);

	printf("Options:\n");
	printf("  -c <count>    : Set the number of pings to send\n");
	printf("  -i <interval> : Set the interval between pings\n");
	printf("  -p <pattern>  : Set the pattern to send in the payload\n");
	printf("  -t <timeout>  : Set the timeout for each ping\n");
	printf("  -V            : Print the version\n");
	printf("  -h -?         : Print the help\n");
}

void	version(void)
{
	AUTO_LOG;
	LOG(GREEN "ft_ping -- ldalmass -- version: 1.0.0" RESET);
}

void	init_ping_struct(t_ping *ping, char **argv)
{
	AUTO_LOG;

	ping->program_name = argv[0];
	ping->is_bonus = (strstr(argv[0], "ft_ping_bonus") == NULL) ? false : true;
	ping->is_root = (getuid() == 0);
	ping->is_flooding = false;
	ping->hostname = NULL;
	ping->ip_str = NULL;
	ping->addr_info = NULL;
	ping->interval = 1;
	ping->timeout = 0;
	ping->ttl = 64;
	ping->preload_count = 0;
	ping->payload_length = PING_DEFAULT_DATA_LEN;
	ping->ip = 0;
	ping->count = -1;
	ping->packet = NULL;
	ping->packet_len = sizeof(t_icmp_header) + ping->payload_length;
	ping->packet_sent_count = 0;
	ping->packet_recieved_count = 0;
}

int parse_args(int argc, char **argv, t_ping *ping)
{
	AUTO_LOG;

	int opt = 0;
	// Parse all the arguments
	while (optind < argc)
	{
		// Checks for options
		while ((opt = getopt(argc, argv, "?wlhfvVc:i:p:r:")) != -1)
		{
			switch (opt)
			{
				case 'c':
					ping->count = atoi(optarg);
					LOG(BLUE "count: %d" RESET, ping->count);
					if (ping->count <= 0)
						return (LOG(RED "Error: Count must be greater than 0" RESET), help(argv[0]), EXIT_FAILURE);
					break;
				case 'i':
					ping->interval = atof(optarg);
					LOG(BLUE "interval: %f" RESET, ping->interval);
					if (ping->interval < 0.2)
						return (LOG(RED "Error: Interval must be greater than 0.2 seconds" RESET), help(argv[0]), EXIT_FAILURE);
					break;
				case 'p':
					ping->payload_length = strlen(optarg);
					ping->payload_raw_string = optarg;
					ping->packet_len = (sizeof(t_icmp_header)) + ping->payload_length;
					break;
				case 'r':
					if (atoi(optarg) <= 0 || atoi(optarg) > 255)
						return (LOG(RED "Error: Time To Live (TTL) must be between 1 and 255" RESET), help(argv[0]), EXIT_FAILURE);
					ping->ttl = atoi(optarg);
					break;
				case 'w':
					ping->timeout = atoi(optarg);
					break;
				case 'f':
					if (!ping->is_root) return (LOG(RED "Error: Flooding require root privileges" RESET), help(argv[0]), EXIT_FAILURE);
					ping->is_flooding = true;
					break;
				case 'l':
					ping->preload_count = atoi(optarg);
					if (ping->preload_count > 3 && !ping->is_root)
						return (LOG(RED "Error: Preload option requires root privileges" RESET), help(argv[0]), EXIT_FAILURE);
					break;
				case 'v': // to do: Verbose output. Do not suppress DUP replies when pinging multicast address.
					return (version(), EXIT_FAILURE);
				case 'V':
					return (version(), EXIT_FAILURE);
				case 'h':
					return (help(argv[0]), EXIT_FAILURE);
				case '?':
					return (help(argv[0]), EXIT_FAILURE);
				default:
					return (help(argv[0]), EXIT_FAILURE);
			}
		}
		// Checks for standalone options
		if (ping->hostname == NULL)
			ping->hostname = argv[optind++];
		else if (ping->hostname != NULL && argv[optind] != NULL)
			return (LOG(RED "Error: Multiple hostnames provided" RESET), help(argv[0]), EXIT_FAILURE);
	}
	// Check if no hostname is provided
	if (ping->hostname == NULL)
		return (LOG(RED "Error: No hostname provided" RESET), help(argv[0]), EXIT_FAILURE);
	return (EXIT_SUCCESS);
}
