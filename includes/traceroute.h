/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_traceroute.h                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:10:35 by ldalmass          #+#    #+#             */
/*   Updated: 2026/04/28 1919:2020:3737 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <getopt.h>

#include <arpa/inet.h>
#include <limits.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include <netinet/ip_icmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "log.h"
#include "colors.h"

#define unused __attribute__((unused))

// TO REMOVE LATER -- C LANGUAGE SERVER DOESNT SUPPORT C23
#define true 1
#define false 0
#define bool int
// END TO REMOVE LATER

#define PING_DEFAULT_DATA_LEN 56
#define NI_MAXHOST 1025

extern volatile bool g_is_running;

typedef struct s_icmp_header
{
	// struct s_echo_header	header;

	// First 32 bits (8 + 8 + 16)
	uint8_t type;
	uint8_t code;
	uint16_t checksum;

	// Second 32 bits (16 + 16)
	uint16_t identifier;
	uint16_t sequence_number;
} t_icmp_header;

typedef struct s_replies
{
	struct s_icmp_header reply;
	struct s_replies *next;

	char reversed_dns_str[NI_MAXHOST];

	uint32_t	length;
	// uint32_t	offset;
	uint32_t	reversed_ttl;
	uint32_t	reversed_ip;
	char		*reversed_ip_str;

	float elapsed_time_in_seconds;
	float elapsed_time_in_usec;
	float elapsed_time_in_ms;
} t_replies;

typedef struct s_tr
{
	struct s_icmp_header icmp_packet;
	struct addrinfo *addr_info;
	struct s_replies *replies;
	struct timeval total_time_elapsed;

	bool exit_status;
	// bool		is_flooding;
	// bool		is_verbose;
	// bool		is_quiet;
	// char *payload_raw_string;
	// uint32_t	packet_sent_count;
	// uint32_t	packet_recieved_count;
	// int			linger;
	// int			count;
	int sequence;
	// int preload_count;
	uint8_t *packet;
	size_t packet_len;

	int			sockfd;
	bool		is_bonus;
	bool		is_root;
	bool		do_reverse_dns;
	char		*program_name;
	char		*hostname;
	char		*ip_str;
	uint32_t	payload_length;
	uint32_t	ip;
	uint32_t	tos;
	uint32_t	ttl;
	uint32_t	max_hops;
	uint32_t	offset_hop;
	uint32_t	probes_per_hop;
	int			response_timeout_for_each_probe;
	float		interval;
} t_tr;

// parser.c
int parse_args(int argc, char **argv, t_tr *tr);
void help(t_tr *tr);
void init_traceroute_struct(t_tr *tr, char **argv);

// socket.c
int create_icmp_socket(t_tr *tr);
int resolve_hostname(t_tr *tr);
void find_the_ip(t_tr *tr);
void get_sockaddr(struct sockaddr_in *ai_addr, t_tr *tr);
void send_packet(t_tr *tr);
char *transform_raw_ip_to_string_ip(const unsigned int ip);

// icmp_packet.c
uint16_t calculate_checksum(void *addr, int count);
void init_icmp_header(t_tr *tr);
void read_payload_data_in_packet(t_tr *tr);
void build_traceroute_packet(t_tr *tr);
void add_payload_to_packet(t_tr *tr);
void serialize_icmp_packet(t_tr *tr);
float deserialize_icmp_packet(t_tr *tr, struct timeval start);

// utils.c
struct timeval get_time();
bool did_we_timeout(struct timeval start, t_tr *tr);
bool did_we_exceed_in_seconds(struct timeval start, uint32_t seconds);
void handle_sigint(int signum unused);
