/*
 * log.c
 *
 *  Created on: Feb 1, 2017
 *      Author: pchero
 */

#include <syslog.h>
#include <stdbool.h>
#include <jansson.h>

#include "common.h"

#define PROGNAME "jade"
#define DEF_LOGLEVEL 3

extern app* g_app;

bool g_log_initiated = false;


bool init_log(void)
{
  // set max loglevel
  setlogmask(LOG_UPTO(DEF_LOGLEVEL));

  // set log
  openlog(PROGNAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  printf("Complete init_log.\n");

  return true;
}

bool update_log_level(int level)
{
  setlogmask(LOG_UPTO(level));

  return true;
}

