/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 13:35:26 by ldalmass          #+#    #+#             */
/*   Updated: 2026/01/09 13:52:28 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/log.h"

static int depth = 0;

t_log *log_init(const char *func_name)
{
	t_log *log;
	size_t len;

	if (!func_name)
		return (NULL);
	len = strlen(func_name);
	log = (t_log *)malloc(sizeof(t_log));
	if (!log)
		return (NULL);
	log->function_name = (char *)malloc(len + 1);
	if (!log->function_name)
	{
		free(log);
		return (NULL);
	}
	strcpy(log->function_name, func_name);
	depth++;
	for (int i = 1; i < depth; i++)
		printf("%s", WALL);
	printf("%s%s%s\n", IN, log->function_name, TRAIL);
	return (log);
}

void log_cleanup(t_log **log)
{
	if (!log || !*log)
		return;
	for (int i = 1; i < depth; i++)
		printf("%s", WALL);
	printf("%s%s%s\n", OUT, (*log)->function_name, TRAIL);
	depth--;
	if ((*log)->function_name)
		free((*log)->function_name);
	free(*log);
	*log = NULL;
}

void log_message(t_log *log, const char *message)
{
	if (!log || !message)
		return;
	for (int i = 0; i < depth; i++)
		printf("%s", WALL);
	printf("%s\n", message);
}

void log_message_v(t_log *log, const char *format, ...)
{
	va_list args;
	char buffer[1024];

	if (!log || !format)
		return;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	for (int i = 0; i < depth; i++)
		printf("%s", WALL);
	printf("%s\n", buffer);
}