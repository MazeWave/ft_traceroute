/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_ping.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 11:27:26 by ldalmass          #+#    #+#             */
/*   Updated: 2026/03/23 17:51:04 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FT_PING_H
#define FT_PING_H

# include <stddef.h>
# include <string.h>
# include <stdio.h>
# include <unistd.h>
# include <stdlib.h>

# include <arpa/inet.h>
# include <fcntl.h>
# include <limits.h>
# include <math.h>
# include <netdb.h>
# include <netinet/in.h>
# include <netinet/ip_icmp.h>
# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>

# include "log.h"
# include "colors.h"

#define unused __attribute__((unused))

// TO REMOVE LATER
#define true 1
#define false 0
#define bool int
// END TO REMOVE LATER

// From lpolizzi ft_ping
# define SECOND_IN_USEC 1000000

# define MAXSEQ 65535

# define MAXIPLEN 60
# define MAXICMPLEN 76

# define PING_DEFAULT_DATA_LEN 56 // Can be found in the man page of ping
# define PING_MAX_DATA_LEN (65507 - MAXIPLEN - MAXICMPLEN)
// end of lpolizzi ft_ping

extern volatile bool	g_is_running;

typedef struct	s_echo_header
{
	// First 32 bits
	uint8_t		type;
	uint8_t		code;
	uint16_t	checksum;

	// Second 32 bits
	uint16_t	identifier;
	uint16_t	sequence_number;
}	t_echo_header;

typedef struct	s_echo_payload
{
	// Payload (32 bits * x times needed)
	uint32_t	*data;
	size_t		length;
}	t_echo_payload;

typedef struct s_icmp_packet
{
	t_echo_header	header;
	t_echo_payload	payload;
}	t_icmp_packet;

typedef struct s_ping
{
	struct s_icmp_packet	icmp_packet;
	// struct s_icmp_packet	*icmp_replies;
	struct addrinfo			*addr_info;

	bool		is_bonus;
	bool		is_root;
	bool		is_flooding;
	char		*program_name;
	char		*hostname;
	char		*ip_str;
	char		*payload_raw_string;
	uint32_t	payload_length;
	uint32_t	ip;
	uint8_t		ttl;
	int			count;
	int			preload_count;
	int			timeout;
	int			sockfd;
	float		interval;
}	t_ping;

// parser.c
int		parse_args(int argc, char **argv, t_ping *ping);
void	help(char *elf_name);
void	init_ping_struct(t_ping *ping, char **argv);
void	print_ping_struct(t_ping *ping);
void	print_packet_informations(t_ping *ping);

// socket.c
int		create_icmp_socket(t_ping *ping);
int		resolve_hostname(t_ping *ping);
void	print_addr_info(t_ping *ping);
void	print_sockaddr(struct sockaddr_in *ai_addr, t_ping *ping);
void	send_ping(t_ping *ping);

// icmp_packet.c
uint16_t	calculate_checksum(uint16_t *addr, int count);
void		init_echo_header(t_ping *ping);
void		read_payload_data_in_packet(t_ping *ping);
void 		build_ping_packet(t_ping *ping);
void		add_payload_to_packet(t_ping *ping);

#endif
