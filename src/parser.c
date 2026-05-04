/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:11:05 by ldalmass          #+#    #+#             */
/*   Updated: 2026/04/29 16:39:02 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"

void get_sockaddr(struct sockaddr_in *ai_addr, t_tr *tr)
{
	AUTO_LOG;

	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ai_addr->sin_addr, ip_str, INET_ADDRSTRLEN);

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
	// if (tr->is_root && tr->is_verbose) printf(YELLOW "ai->ai_family: AF_INET,
	// ai->ai_canonname: '%s'\n" RESET, tr->hostname);
	return;
}

void find_the_ip(t_tr *tr)
{
	AUTO_LOG;

	struct addrinfo *temp = tr->addr_info;
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
		get_sockaddr((struct sockaddr_in *)temp->ai_addr, tr);
		LOG("ai_canonname: %s", temp->ai_canonname);
		LOG(RESET);
		temp = temp->ai_next;
	}
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
		printf("  -m            : Set maximal hop count (default: 64)\n");
		printf("  -q            : Send NUM probe packets per hop (default: 3)\n");
		printf("  -w            : Wait NUM seconds for response (default: 3)\n");
		printf("  -f            : Set initial hop distance, i.e., time-to-live\n");
		printf("  -p            : Use destination PORT port (default: 33434)\n");
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
	LOG(GREEN "ft_ping -- ldalmass -- version: 7.7.7" RESET);
}

void init_ping_struct(t_tr *tr, char **argv)
{
	AUTO_LOG;

	// tr->is_quiet = false;
	// tr->is_flooding = false;
	// tr->payload_raw_string = NULL;
	// tr->is_verbose = false;
	// tr->linger = -1;
	// tr->preload_count = 0;
	// tr->payload_length = PING_DEFAULT_DATA_LEN;
	// tr->packet_sent_count = 0;
	// tr->packet_recieved_count = 0;
	tr->program_name = argv[0];
	tr->is_bonus = (strstr(argv[0], "_bonus") == NULL) ? false : true;
	tr->is_root = (getuid() == 0);
	tr->exit_status = false;
	tr->hostname = NULL;
	tr->ip_str = NULL;
	tr->addr_info = NULL;
	tr->replies = NULL;
	tr->interval = 3;
	tr->response_time = 3;
	tr->ttl = 1;
	tr->max_hops = 64;
	tr->offset_hop = 0;
	tr->port = 33434;
	tr->ip = 0;
	tr->count = -1;
	tr->packet = NULL;
	tr->packet_len = sizeof(t_icmp_header) + PING_DEFAULT_DATA_LEN;
	tr->probes_per_hop = 3;
	gettimeofday(&tr->total_time_elapsed, NULL);
}

int parse_args(int argc, char **argv, t_tr *tr)
{
	AUTO_LOG;
	int opt = 0;
	// static bool has_already_printed_error = false;
	LOG(DEBUG RED "optind: %d, argc: %d" RESET, optind, argc);
	while ((opt = getopt(argc, argv, "-h?m:q:w:f:p:")) != -1)
	{
		LOG(DEBUG RED "optind: %d, argc: %d" RESET, optind, argc);
		switch (opt)
		{
		case 'm':
			if (!tr->is_bonus) return (help(tr), tr->exit_status = true);
			tr->max_hops = atoi(optarg);
			LOG(BLUE "max_hops: %d" RESET, tr->count);
			if (tr->max_hops <= 0) return (printf(RED "Error: Max hop must be greater than 0\n" RESET), help(tr), tr->exit_status = true);
			break;
		case 'q':
			if (!tr->is_bonus) return (help(tr), tr->exit_status = true);
			tr->probes_per_hop = atof(optarg);
			LOG(BLUE "probes_per_hop: %f" RESET, tr->interval);
			if (atof(optarg) < 0) return (printf(RED "Error: At least one proble per hop is requiered\n" RESET),help(tr), tr->exit_status = true);
			break;
		case 'w':
			if (!tr->is_bonus) return (help(tr), tr->exit_status = true);
			if (atoi(optarg) <= 0) return (printf(RED "Error: Response time must be at 1 least seconds\n" RESET), help(tr), tr->exit_status = true);
			tr->response_time = atoi(optarg);
			LOG(BLUE "response_time: %d" RESET, tr->response_time);
			break;
		case 'f':
			if (!tr->is_bonus) return (help(tr), tr->exit_status = true);
			if (atoi(optarg) <= 0 || atoi(optarg) > 64) return (printf(RED "Error: The hop offset must be between 0 and 64\n" RESET), help(tr), tr->exit_status = true);
			tr->offset_hop = atoi(optarg);
			LOG(BLUE "offset_hop: %d" RESET, tr->ttl);
			break;
		case 'l':
			if (!tr->is_bonus) return (help(tr), tr->exit_status = true);
			tr->preload_count = atoi(optarg);
			if (tr->preload_count > 3 && !tr->is_root) return (printf(RED "Error: Preload option requires root privileges\n" RESET), help(tr), tr->exit_status = true);
			LOG(BLUE "preload_count: %d" RESET, tr->preload_count);
			break;
		case 'h':
			return (help(tr), tr->exit_status = false, EXIT_FAILURE);
		case '?':
			return (help(tr), tr->exit_status = false, EXIT_FAILURE);
		default:
			if (tr->hostname == NULL) tr->hostname = optarg;
			// else if (tr->is_verbose)
			// 	if (!has_already_printed_error++) printf(RED "Error: Multiple hostnames provided. Only the first one will be used.\n" RESET);
			LOG(RED "Used Hostname: %s" RESET, tr->hostname);
			LOG(RED "Current read Hostname: %s" RESET, optarg);
			break;
		}
	}
	return (EXIT_SUCCESS);
}
