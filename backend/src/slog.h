/*
 * log.h
 *
 *  Created on: Feb 1, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_SLOG_H_
#define BACKEND_SRC_SLOG_H_

#include <stdbool.h>
#include <syslog.h>

#define LOG_EMERG 0   /* system is unusable */
#define LOG_ALERT 1   /* action must be taken immediately */
#define LOG_CRIT  2   /* critical conditions */
#define LOG_ERR   3   /* error conditions */
#define LOG_WARNING 4 /* warning conditions */
#define LOG_NOTICE  5 /* normal but significant condition */
#define LOG_INFO  6   /* informational */
#define LOG_DEBUG 7   /* debug-level messages */


bool init_log(void);

#define slog(...) syslog(__VA_ARGS__)

#endif /* BACKEND_SRC_LOG_H_ */
