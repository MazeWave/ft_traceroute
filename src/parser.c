/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:11:05 by ldalmass          #+#    #+#             */
/*   Updated: 2026/05/15 15:03:21 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"
#include <bits/getopt_core.h>
#include <stdlib.h>

void get_sockaddr(struct sockaddr_in *ai_addr, t_tr *tr)
{
	AUTO_LOG;

	// char ip_str[INET_ADDRSTRLEN];
	// inet_ntop(AF_INET, &ai_addr->sin_addr, ip_str, INET_ADDRSTRLEN);
	char *ip_str = inet_ntoa(ai_addr->sin_addr);

	// set the ip
	tr->ip = ai_addr->sin_addr.s_addr;
	if (tr->ip_str)
		free(tr->ip_str);
	tr->ip_str = strndup(ip_str, INET_ADDRSTRLEN);
	if (!tr->ip_str)
	{
		LOG(RED "%s: malloc: Failed to allocate memory for ip string.\n" RESET,
		tr->program_name);
		tr->ip_str = NULL;
		return;
	}
	LOG(GREEN "ip as int: %d" BLUE, tr->ip);
	LOG(GREEN "ip as string: %s" BLUE, ip_str);
	return;
}

void find_the_ip(t_tr *tr)
{
	AUTO_LOG;

	struct addrinfo *temp = tr->addr_info;
	LOG(BLUE);
	LOG("ai_family: %d", temp->ai_family);
	LOG("ai_socktype: %d", temp->ai_socktype);
	LOG("ai_protocol: %d", temp->ai_protocol);
	LOG("ai_addrlen: %d", temp->ai_addrlen);
	LOG("ai_addr: %p", temp->ai_addr);
	get_sockaddr((struct sockaddr_in *)temp->ai_addr, tr);
	LOG("ai_canonname: %s", temp->ai_canonname);
	LOG(RESET);
	return;
}

void help(t_tr *tr)
{
	AUTO_LOG;

	switch (tr->is_bonus)
	{
	case true:
		printf(GREEN "Usage: %s <hostname> [options]\n" RESET, tr->program_name);

		printf("Options:\n");
		printf("  -i            : Set packet protocol to ICMP\n");
		printf("  -m            : Set maximal hop count (default: 64)\n");
		printf("  -q            : Send NUM probe packets per hop (default: 3)\n");
		printf("  -w            : Wait NUM seconds for response (default: 3)\n");
		printf("  -f            : Set initial hop distance, i.e., time-to-live\n");
		printf("  -r            : Displayed resolved hostnames (if possible)\n");
		printf("  -p            : Change the destination port (default: 33434)\n");
		printf("  -t            : Change TOS (Type of Service) to NUM (default: 0)\n");
		printf("                : 0		(Best effort)(default)\n");
		printf("                : 16	(Low delay)\n");
		printf("                : 40	(Low priority data)\n");
		printf("                : 184	(VoIP and real-time audio transmission)\n");
		printf("  -h -?         : Print the help\n");
		return;
	case false:
		printf(GREEN "Usage: %s <hostname> [options]\n" RESET, tr->program_name);

		printf("Options:\n");
		printf("  -h -?         : Print the help\n");
		return;
	}

	return;
}

void version(void)
{
	AUTO_LOG;
	LOG(GREEN "ft_traceroute -- ldalmass -- version: 2" RESET);
	return ;
}

void init_traceroute_struct(t_tr *tr, char **argv)
{
	AUTO_LOG;

	tr->program_name = argv[0];
	tr->is_bonus = (strstr(argv[0], "_bonus") == NULL) ? false : true;
	tr->is_root = (getuid() == 0);
	tr->is_icmp = false;
	tr->exit_status = false;
	tr->do_reverse_dns = false;
	tr->hostname = NULL;
	tr->ip_str = NULL;
	tr->addr_info = NULL;
	tr->replies = NULL;
	tr->interval = 3;
	tr->response_timeout_for_each_probe = 3;
	tr->ttl = 1;
	tr->max_hops = 64;
	tr->offset_hop = 0;
	tr->tos = 0;
	tr->ip = 0;
	tr->port = DEFAULT_SEND_PORT;
	tr->send_sockfd = -1;
	tr->recv_sockfd = -1;
	tr->packet = NULL;
	tr->packet_len = sizeof(t_udp_header) + PING_DEFAULT_DATA_LEN;
	tr->probes_per_hop = 3;
	gettimeofday(&tr->total_time_elapsed, NULL);

	return ;
}

int parse_args(int argc, char **argv, t_tr *tr)
{
	AUTO_LOG;
	int opt = 0;
	LOG(DEBUG BLUE "optind: %d, argc: %d" RESET, optind, argc);
	while ((opt = getopt(argc, argv, "-h?virm:q:w:f:t:p:")) != -1)
	{
		LOG(DEBUG BLUE "optind: %d, argc: %d" RESET, optind, argc);
		switch (opt)
		{
		case 'r':
			if (!tr->is_bonus) return (help(tr), EXIT_FAILURE);
			tr->do_reverse_dns = true;
			LOG(BLUE "resolve hostname: %d" RESET, tr->do_reverse_dns);
			break;
		case 'm':
			if (!tr->is_bonus) return (help(tr), EXIT_FAILURE);
			tr->max_hops = atoi(optarg);
			LOG(BLUE "max_hops: %d" RESET, tr->max_hops);
			if (tr->max_hops <= 0 && tr->max_hops > 99) return (printf(RED "Error: Max hop must be between 1 and 99\n" RESET),
			help(tr), EXIT_FAILURE);
			break;
		case 'q':
			if (!tr->is_bonus) return (help(tr), EXIT_FAILURE);
			if (atoi(optarg) < 0 || atoi(optarg) > 10) return (printf(RED "Error: Probes count per hop must be between 1 and 10\n" RESET),help(tr), EXIT_FAILURE);
			tr->probes_per_hop = atoi(optarg);
			LOG(BLUE "probes_per_hop: %f" RESET, tr->probes_per_hop);
			break;
		case 'w':
			if (!tr->is_bonus)
			return (help(tr), EXIT_FAILURE);
			if (atoi(optarg) <= 0) return (printf(RED "Error: Response time must be at 1 least seconds\n" RESET), help(tr), EXIT_FAILURE);
			if (atoi(optarg) > 60) return (printf(RED "Error: Ridiculous waiting time `%d'\n" RESET, atoi(optarg)), help(tr), EXIT_FAILURE);
			tr->response_timeout_for_each_probe = atoi(optarg);
			LOG(BLUE "response_time: %d" RESET, tr->response_timeout_for_each_probe);
			break;
		case 'f':
			if (!tr->is_bonus) return (help(tr), EXIT_FAILURE);
			if (atoi(optarg) <= 0) return (printf(RED "Error: The initial hop distance must be at least 1\n" RESET), help(tr), EXIT_FAILURE);
			tr->offset_hop = atoi(optarg);
			LOG(BLUE "offset_hop: %d" RESET, tr->offset_hop);
			break;
		case 't':
			if (!tr->is_bonus)
			return (help(tr), EXIT_FAILURE);
			if (atoi(optarg) <= 0 || atoi(optarg) > 255) return (printf(RED "Error: The TOS must be between 0 and 255\n" RESET), help(tr), EXIT_FAILURE);
			tr->tos = atoi(optarg);
			LOG(BLUE "tos: %d" RESET, tr->tos);
			break;
		case 'i':
			if (!tr->is_bonus)
			return (help(tr), EXIT_FAILURE);
			tr->is_icmp = true;
			tr->packet_len = sizeof(t_icmp_header) + PING_DEFAULT_DATA_LEN;
			LOG(BLUE "is_icmp: %d" RESET, tr->is_icmp);
			break;
		case 'p':
			if (!tr->is_bonus)
			return (help(tr), EXIT_FAILURE);
			if (atoi(optarg) < 1 || atoi(optarg) > 65535) return (printf(RED "Error: The port must be between 1 and 65535\n" RESET), help(tr), EXIT_FAILURE);
			tr->port = atoi(optarg);
			LOG(BLUE "port: %d" RESET, tr->port);
			break;

		case 'v':
			return (version(), tr->exit_status = false, EXIT_FAILURE);
		case 'h':
			return (help(tr), tr->exit_status = false, EXIT_FAILURE);
		case '?':
			return (help(tr), tr->exit_status = false, EXIT_FAILURE);
			default:
			if (tr->hostname == NULL) tr->hostname = optarg;
			LOG(BLUE "Used Hostname: %s" RESET, tr->hostname);
			LOG(BLUE "Current read Hostname: %s" RESET, optarg);
			break;
		}
	}

	// Port validation maths
	int max_port = (tr->port + (tr->max_hops * tr->probes_per_hop) - 1); 
	if (max_port > 65535)
		return (printf(RED "Error: the port must not exceed 65535 when hopping for %d times, each hop increment the port's value" RESET, max_port), EXIT_FAILURE);
	return (EXIT_SUCCESS);
}
