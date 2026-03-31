/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:27:26 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/25 1818:4242:3030 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
#define FT_PING_H

# include <stddef.h>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>

# include <bits/getopt_core.h>

# include <arpa/inet.h>
# include <fcntl.h>
# include <limits.h>
# include <math.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <netinet/ip.h>
# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>
# include <time.h>

# include <netinet/ip_icmp.h>
# include <stddef.h>
# include <stdint.h>
# include <stdlib.h>
# include <math.h>

# include "log.h"
# include "colors.h"

# define unused __attribute__((unused))

// TO REMOVE LATER -- C LANGUAGE SERVER DOESNT SUPPORT C23
#define true 1
#define false 0
#define bool int
// END TO REMOVE LATER

// From lpolizzi ft_ping
// # define MAXSEQ 65535
// # define MAXIPLEN 60
// # define MAXICMPLEN 76
// # define PING_MAX_DATA_LEN (65507 - MAXIPLEN - MAXICMPLEN)
// end of lpolizzi ft_ping

# define PING_DEFAULT_DATA_LEN 56
# define NI_MAXHOST 1025

extern volatile bool	g_is_running;

typedef struct	s_icmp_header
{
	// struct s_echo_header	header;
	
	// First 32 bits (8 + 8 + 16)
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;

	// Second 32 bits (16 + 16)
	uint16_t	identifier;
	uint16_t	sequence_number;
}	t_icmp_header;

typedef struct s_replies
{
	struct s_icmp_header	reply;
	struct s_replies		*next;
	
	char					reversed_dns_str[NI_MAXHOST];
	
	unsigned int			length;
	unsigned int			offset;
	unsigned int			ttl;
	
	float					elapsed_time_in_seconds;
	float					elapsed_time_in_usec;
	float					elapsed_time_in_ms;
}	t_replies;

typedef struct	s_ping
{
	struct s_icmp_header	icmp_packet;
	struct addrinfo			*addr_info;
	struct s_replies		*replies;
	struct timeval			total_time_elapsed;

	bool		is_bonus;
	bool		is_root;
	bool		is_flooding;
	bool		is_verbose;
	bool		exit_status;
	char		*program_name;
	char		*hostname;
	char		*ip_str;
	char		*payload_raw_string;
	uint32_t	payload_length;
	uint32_t	ip;
	uint8_t		ttl;
	int			sequence;
	int			count;
	int			preload_count;
	uint32_t	packet_sent_count;
	uint32_t	packet_recieved_count;
	int			timeout;
	int			sockfd;
	float		interval;
	uint8_t		*packet;
	size_t		packet_len;
	
}	t_ping;

// parser.c
int		parse_args(int argc, char **argv, t_ping *ping);
void	help(t_ping *ping);
void	init_ping_struct(t_ping *ping, char **argv);
void	print_ping_struct(t_ping *ping);
void	print_packet_informations(t_ping *ping);
void	print_bits(uint32_t n);
void	print_echo_reply(t_ping *ping);

// socket.c
int		create_icmp_socket(t_ping *ping);
int		resolve_hostname(t_ping *ping);
void	find_the_ip(t_ping *ping);
void	get_sockaddr(struct sockaddr_in *ai_addr, t_ping *ping);
void	send_ping(t_ping *ping);

// icmp_packet.c
uint16_t	calculate_checksum(void *addr, int count);
void		init_icmp_header(t_ping *ping);
void		read_payload_data_in_packet(t_ping *ping);
void 		build_ping_packet(t_ping *ping);
void		add_payload_to_packet(t_ping *ping);
void		serialize_icmp_packet(t_ping *ping);
float		deserialize_icmp_packet(t_ping *ping, struct timeval start);

#endif
