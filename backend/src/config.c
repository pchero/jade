/*
 * config.c
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#include <stdbool.h>
#include <jansson.h>

#include "slog.h"

#define DEF_CONF_FILENAME "./jade.conf"

extern struct json_t* g_app;

static bool load_config(void);


bool init_config(void)
{
  int ret;

  ret = load_config();
  if(ret == false) {
    return false;
  }

  return true;
}

static bool load_config(void)
{
  json_t* j_conf;
  json_error_t j_err;

  slog(LOG_INFO, "Load config file. filename[%s]", DEF_CONF_FILENAME);

  j_conf = json_load_file(DEF_CONF_FILENAME, JSON_DECODE_ANY, &j_err);

  if(g_app != NULL) {
    json_decref(g_app);
  }

  g_app = j_conf;

  return true;
}
