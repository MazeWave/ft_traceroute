/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 14:35:54 by ldalmass          #+#    #+#             */
/*   Updated: 2026/01/16 16:18:51 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ft_ping.h"

void    print_ping_struct(t_ping *ping)
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

void	print_sockaddr(struct sockaddr_in *ai_addr, t_ping *ping)
{
	AUTO_LOG;
	
	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ai_addr->sin_addr, ip_str, INET_ADDRSTRLEN);
	
	// set the ip
	ping->ip = ai_addr->sin_addr.s_addr;
	ping->ip_str = ip_str;
	LOG(GREEN "ip as int: %d" BLUE, ping->ip);
	LOG(GREEN "ip as string: %s" BLUE, ip_str);
}

void	print_addr_info(t_ping *ping)
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
		print_sockaddr((struct sockaddr_in *)temp->ai_addr, ping);
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
	printf("  -t <interval> : Set the interval between pings\n");
	printf("  -v            : Print the version\n");
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
	ping->hostname = NULL;
	// ping->payload = NULL;
	ping->ip_str = NULL;
	ping->addr_info = NULL;
	ping->icmp_packet.header = init_echo_header();
	ping->interval = 1;
	// ping->payload_length = 0;
	ping->ip = 0;
	ping->count = -1;
}

int parse_args(int argc, char **argv, t_ping *ping)
{
	AUTO_LOG;

	int opt = 0;
	// Parse all the arguments
	while (optind < argc)
	{
		// Checks for options
		while ((opt = getopt(argc, argv, "?hvc:i:p:t:")) != -1)
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
					// if (optarg[0] == '-')
					// 	return (LOG(RED "Error: Pattern cannot be empty" RESET), help(argv[0]), EXIT_FAILURE);
					ping->payload_length = strlen(optarg);
					ping->payload_raw_string = optarg;
					break;
				case 't':
					ping->interval = atof(optarg);
					break;
				case 'v':
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