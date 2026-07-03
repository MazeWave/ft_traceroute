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
