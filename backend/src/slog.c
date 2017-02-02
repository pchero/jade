/*
 * log.c
 *
 *  Created on: Feb 1, 2017
 *      Author: pchero
 */

#include <syslog.h>
#include <stdbool.h>
#include <jansson.h>

extern struct json_t* g_app;

#define PROGNAME "jade"

bool init_log(void)
{
  const char* tmp_const;
  int level;

  tmp_const = json_string_value(json_object_get(g_app, "loglevel"));
  if(tmp_const == NULL) {
    return false;
  }

  // set max loglevel
  level = atoi(tmp_const);
  setlogmask(LOG_UPTO(level));

  // set log
  openlog (PROGNAME, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
  printf("Complete init_log.\n");

  return true;
}
