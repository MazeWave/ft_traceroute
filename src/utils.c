/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 17:10:44 by ldalmass          #+#    #+#             */
/*   Updated: 2026/0505/1313 14:3232:5656 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/traceroute.h"
#include <bits/types/error_t.h>

struct timeval get_time()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	return (time);
}

bool did_we_timeout(struct timeval start, t_tr *tr)
{
	if (tr->response_timeout_for_each_probe == -1) return (false);
	struct timeval end = get_time();
	int elapsed_time_in_sec = end.tv_sec - start.tv_sec;
	bool did_we_timeout = (elapsed_time_in_sec >= tr->response_timeout_for_each_probe) ? true : false;
	return (did_we_timeout);
}

bool did_we_exceed_in_seconds(struct timeval start, uint32_t seconds)
{
	struct timeval end = get_time();
	int elapsed_time_in_sec = end.tv_sec - start.tv_sec;
	bool did_we_timeout = ((uint32_t)elapsed_time_in_sec >= seconds) ? true : false;
	return (did_we_timeout);
}

void handle_sigint(int signum unused)
{
	AUTO_LOG;

	LOG(YELLOW "signal %d received, stopping traceroute" RESET, signum);
	g_is_running = false;
	return;
}

void print_errno(void)
{
	AUTO_LOG;

	switch (errno) {
		case EACCES:
			LOG(BG_RED " EACCES " RESET);
			LOG(RED "(For UNIX domain sockets, which are identified by pathname permission is denied on the destination socket file, or search is  for one of the  directo‐ries the path prefix.  (See path_resolution(7).)(For UDP sockets) An attempt was made to send to a network/ address as though it was a unicast address." RESET);
			break;
		case EAGAIN | EWOULDBLOCK:
			LOG(BG_RED " EAGAIN  | EWOULDBLOCK" RESET);
			LOG(RED "The socket is marked nonblocking and the requested operation would block.  POSIX.1-2001 allows either error to be returned for this case, and does not require these constants to have the same value, so a portable application should check for possibilities. (Internet  domain  datagram sockets) The socket referred to by sockfd had not previously been bound to an address and, upon attempting to bind it to an ephemeral port, it was determined that all port numbers in the ephemeral port range  currently in use. the discussion of /proc/sys/net/ /ip_local_port_range in ip(7)." RESET);
			break;
		case EALREADY:
			LOG(BG_RED " EALREADY " RESET);
			LOG(RED "Another Fast Open is in progress." RESET);
			break;
		case EBADF:
			LOG(BG_RED " EBADF " RESET);
			LOG(RED "sockfd is not a valid open file descriptor." RESET);
			break;
		case ECONNRESET:
			LOG(BG_RED " ECONNRESET " RESET);
			LOG(RED "Connection reset by peer." RESET);
			break;
		case EDESTADDRREQ:
			LOG(BG_RED " EDESTADDRREQ " RESET);
			LOG(RED "The socket is not connection-mode, and no peer address is set." RESET);
			break;
		case EFAULT:
			LOG(BG_RED " EFAULT " RESET);
			LOG(RED "An invalid user space address was specified for an argument." RESET);
			break;
		case EINTR:
			LOG(BG_RED " EINTR " RESET);
			LOG(RED "A signal occurred before any data was transmitted; see signal(7)." RESET);
			break;
		case EINVAL:
			LOG(BG_RED " EINVAL " RESET);
			LOG(RED "Invalid argument passed." RESET);
			break;
		case EISCONN:
			LOG(BG_RED " EISCONN " RESET);
			LOG(RED "The connection-mode socket was connected already but a recipient was specified.  (Now either this error is returned, or the recipient specification is ignored.)" RESET);
			break;
		case EMSGSIZE:
			LOG(BG_RED " EMSGSIZE " RESET);
			LOG(RED "The socket type requires that message be sent atomically, and the size of the message to be sent made this impossible." RESET);
			break;
		case ENOBUFS:
			LOG(BG_RED " ENOBUFS " RESET);
			LOG(RED "The output queue for a network interface was full.  This generally indicates that the interface has stopped sending, but may be caused by  transient  congestion.   (Normally, this does not occur in Linux.  Packets are just silently dropped when device queue overflows.)" RESET);
			break;
		case ENOMEM:
			LOG(BG_RED " ENOMEM " RESET);
			LOG(RED "No memory available." RESET);
			break;
		case ENOTCONN:
			LOG(BG_RED " ENOTCONN " RESET);
			LOG(RED "The socket is not connected, and no target has been given." RESET);
			break;
		case ENOTSOCK:
			LOG(BG_RED " ENOTSOCK " RESET);
			LOG(RED "The file descriptor sockfd does not refer to a socket." RESET);
			break;
		case EOPNOTSUPP:
			LOG(BG_RED " EOPNOTSUPP " RESET);
			LOG(RED "Some bit in the flags argument is inappropriate for the socket type." RESET);
			break;
		case EPIPE:
			LOG(BG_RED " EPIPE " RESET);
			LOG(RED "The local end has been shut down on a connection oriented socket.  In this case, the process will also receive a SIGPIPE unless MSG_NOSIGNAL is set." RESET);
			break;
	}
}