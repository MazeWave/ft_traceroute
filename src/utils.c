#include "../includes/ft_ping.h"

struct timeval	get_time()
{
	struct timeval	time;
	gettimeofday(&time, NULL);
	return (time);
}

bool	did_we_timeout(struct timeval start, t_ping *ping)
{
	if (ping->timeout == -1) return (false);
	struct timeval	end = get_time();
	int				elapsed_time_in_sec = end.tv_sec - start.tv_sec;
	bool			did_we_timeout = (elapsed_time_in_sec >= ping->timeout) ? true : false;
	return (did_we_timeout);
}

bool	did_we_exceed_in_seconds(struct timeval start, uint32_t seconds)
{
	struct timeval	end = get_time();
	int				elapsed_time_in_sec = end.tv_sec - start.tv_sec;
	bool			did_we_timeout = ((uint32_t)elapsed_time_in_sec >= seconds) ? true : false;
	return (did_we_timeout);
}

void	handle_sigint(int signum unused)
{
	AUTO_LOG;

	LOG(YELLOW "signal %d received, stopping ping" RESET, signum);
	g_is_running = false;
	return;
}

void	print_end_statistics(t_ping *ping)
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
	if (isnan(round_trip_ecart_type)) round_trip_ecart_type = 0;

	ping->total_time_elapsed = get_time();
	printf(YELLOW "\n--- %s ping statistics ---\n" RESET, ping->hostname);
	int	packet_loss = (int)((1.0 - ((float)ping->packet_recieved_count / (float)ping->packet_sent_count)) * 100);
	if (packet_loss == INT_MAX || packet_loss == INT_MIN) packet_loss = 0;
	printf(
		YELLOW "%d packets transmitted, %d packets received, %d%% packet loss\n" RESET,
		ping->packet_sent_count,
		ping->packet_recieved_count,	
		packet_loss
	);
	if (ping->packet_recieved_count == 0 && ping->packet_sent_count > 0) return ;
	printf(
		YELLOW "round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n" RESET,
		round_trip_min,
		round_trip_avg,
		round_trip_max,
		round_trip_ecart_type
	);

	return ;
}

void	print_echo_reply(t_ping *ping)
{
	AUTO_LOG;

	if (!ping->replies)
	{
		LOG(RED "%s: No replies received yet.\n" RESET, ping->program_name);
		return ;
	}
	t_replies	*reply = ping->replies;
	while (reply->next)
		reply = reply->next;

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