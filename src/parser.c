#include "../includes/ft_ping.h"
#include <stdio.h>
#include <stdlib.h>

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
			GREEN "%u bytes from %s (%s): icmp_seq=%d ttl=%d time=%.3f ms\n" RESET,
			reply->length,
			reply->reversed_dns_str,
			ping->ip_str,
			reply->reply.sequence_number,
			reply->ttl,
			reply->elapsed_time_in_ms
		);
		return ;
	case false:
		printf(
			GREEN "%u bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n" RESET,
			reply->length,
			ping->ip_str,
			reply->reply.sequence_number,
			reply->ttl,
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

void	help(t_ping *ping)
{
	AUTO_LOG;

	switch(ping->is_bonus)
	{
		case true:
			printf(GREEN "Usage: %s <hostname> [options]\n" RESET, ping->program_name);
		
			printf("Options:\n");
			printf("  -c <count>    : Set the number of pings to send\n");
			printf("  -i <interval> : Set the interval between pings\n");
			printf("  -p <pattern>  : Set the pattern to send in the payload\n");
			printf("  -s <number>   : Set the packet size to send in bytes\n");
			printf("  -w <timeout>  : Set the timeout for each ping\n");
			printf("  -l <count>    : Preload a set number of packets\n");
			printf("  -r <count>    : Set the Time To Live (TTL)\n");
			printf("  -f            : Floods packet as fast as possible\n");
			printf("  -V            : Print the version\n");
			printf("  -h -?         : Print the help\n");
			return ;
		case false:
			printf(GREEN "Usage: %s <hostname> [options]\n" RESET, ping->program_name);
		
			printf("Options:\n");
			printf("  -v            : Verbose output\n");
			printf("  -h -?         : Print the help\n");
			return ;
	}
	
	return ;
}

void	version(void)
{
	AUTO_LOG;
	LOG(GREEN "ft_ping -- ldalmass -- version: 7.7.7" RESET);
}

void	init_ping_struct(t_ping *ping, char **argv)
{
	AUTO_LOG;

	ping->program_name = argv[0];
	ping->is_bonus = (strstr(argv[0], "ft_ping_bonus") == NULL) ? false : true;
	ping->is_root = (getuid() == 0);
	ping->is_flooding = false;
	ping->exit_status = false;
	ping->hostname = NULL;
	ping->ip_str = NULL;
	ping->addr_info = NULL;
	ping->replies = NULL;
	ping->payload_raw_string = NULL;
	ping->is_verbose = false;
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
	gettimeofday(&ping->total_time_elapsed, NULL);
}

int	parse_args(int argc, char **argv, t_ping *ping)
{
	AUTO_LOG;
	int	opt = 0;
	static bool	has_already_printed_error = false;
	// LOG(DEBUG RED "optind: %d, argc: %d" RESET, optind, argc);
	while ((opt = getopt(argc, argv, "-h?Vvfl:w:r:p:s:i:c:")) != -1)
	{
		// LOG(DEBUG RED "optind: %d, argc: %d" RESET, optind, argc);
		switch (opt)
		{
			case 'c':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				ping->count = atoi(optarg);
				LOG(BLUE "count: %d" RESET, ping->count);
				if (ping->count <= 0) return (printf(RED "Error: Count must be greater than 0\n" RESET), help(ping), ping->exit_status = true);
				break;
			case 'i':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				ping->interval = atof(optarg);
				LOG(BLUE "interval: %f" RESET, ping->interval);
				if (!ping->is_root && ping->interval < 0.2) return (printf(RED "Error: Interval must be greater than 0.2 seconds\n" RESET), help(ping), ping->exit_status = true);
				break;
			case 'p':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				ping->payload_length = strlen(optarg);
				ping->payload_raw_string = optarg;
				ping->packet_len = (sizeof(t_icmp_header)) + ping->payload_length;
				LOG(BLUE "payload_length: %d" RESET, ping->payload_length);
				LOG(BLUE "payload_raw_string: %s" RESET, ping->payload_raw_string);
				LOG(BLUE "packet_len: %d" RESET, ping->packet_len);
				break;
			case 's':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				ping->payload_length = strlen(optarg);
				ping->payload_raw_string = optarg;
				ping->packet_len = (sizeof(t_icmp_header)) + ping->payload_length;
				LOG(BLUE "payload_length: %d" RESET, ping->payload_length);
				LOG(BLUE "payload_raw_string: %s" RESET, ping->payload_raw_string);
				LOG(BLUE "packet_len: %d" RESET, ping->packet_len);
				break;
			case 'r':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				if (atoi(optarg) <= 0 || atoi(optarg) > 255) return (printf(RED "Error: Time To Live (TTL) must be between 1 and 255\n" RESET), help(ping), ping->exit_status = true);
				ping->ttl = atoi(optarg);
				LOG(BLUE "ttl: %d" RESET, ping->ttl);
				break;
			case 'w':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				ping->timeout = atoi(optarg);
				LOG(BLUE "timeout: %d" RESET, ping->timeout);
				break;
			case 'f':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = false, EXIT_FAILURE);
				if (!ping->is_root) return (printf(RED "Error: Flooding require root privileges\n" RESET), help(ping), ping->exit_status = false, EXIT_FAILURE);
				ping->is_flooding = true;
				LOG(BLUE "is_flooding: %d" RESET, ping->is_flooding);
				break;
			case 'l':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				ping->preload_count = atoi(optarg);
				if (ping->preload_count > 3 && !ping->is_root) return (printf(RED "Error: Preload option requires root privileges\n" RESET), help(ping), ping->exit_status = true);
				LOG(BLUE "preload_count: %d" RESET, ping->preload_count);
				break;
			case 'v': // to do: Verbose output. Do not suppress DUP replies when pinging multicast address.
				ping->is_verbose = true;
				break;
			case 'V':
				if (!ping->is_bonus) return(help(ping), ping->exit_status = true);
				return (version(), ping->exit_status = false, EXIT_FAILURE);
			case 'h':
				return (help(ping), ping->exit_status = false, EXIT_FAILURE);
			case '?':
				return (help(ping), ping->exit_status = false, EXIT_FAILURE);
			default:
				
				if (ping->hostname == NULL) ping->hostname = optarg;
				else if (ping->is_verbose)
					if (!has_already_printed_error++) printf(RED "Error: Multiple hostnames provided. Only the first one will be used.\n" RESET);
				LOG(RED "Used Hostname: %s" RESET, ping->hostname);
				LOG(RED "Current read Hostname: %s" RESET, optarg);
				break;
		}
	}
	return (EXIT_SUCCESS);
}