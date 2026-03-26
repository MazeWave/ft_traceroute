/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   log.h                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 13:30:48 by ldalmass          #+#    #+#             */
/*   Updated: 2026/01/09 13:53:18 by ldalmass         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOG_H
# define LOG_H

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <stdarg.h>

# define IN			"┌─ IN  "
# define WALL		"│  "
# define OUT		"└─ OUT "
# define TRAIL		" ───────┄┄╌┈"

typedef struct s_log
{
	char	*function_name;
}	t_log;

// Function declarations
t_log	*log_init(const char *func_name);
void	log_cleanup(t_log **log);
void	log_message(t_log *log, const char *message);
void	log_message_v(t_log *log, const char *format, ...);

// Internal cleanup function for AUTO_LOG macro
static void	_log_cleanup_func(t_log **log) __attribute__((unused));
static void	_log_cleanup_func(t_log **log)
{
	if (log && *log)
		log_cleanup(log);
}


#ifdef PRINT_LOGS
	# define AUTO_LOG \
		t_log *logInstance __attribute__((cleanup(_log_cleanup_func))) = log_init(__FUNCTION__)
	# define LOG(...)	log_message_v(logInstance, __VA_ARGS__)
#else 
	# define AUTO_LOG	((void)0)                     
	# define LOG(...)	((void)0)
#endif

// Macro for automatic logging (creates log instance that auto-cleans up)
// # define AUTO_LOG
	// t_log *logInstance __attribute__((cleanup(_log_cleanup_func))) = log_init(__FUNCTION__)

// Macro for logging messages with printf-style formatting
// # define LOG(...)	log_message_v(logInstance, __VA_ARGS__)

#endif