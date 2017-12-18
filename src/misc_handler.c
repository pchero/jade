/*
 * misc_handler.c
 *
 *  Created on: Dec 17, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "resource_handler.h"

#include "misc_handler.h"

extern app* g_app;

static bool init_modules_info(void);


bool init_misc_handler(void)
{
  int ret;

  ret = init_modules_info();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate module info.");
    return false;
  }

  return true;
}

/**
 * Check installed asterisk modules and update into db.
 * @return
 */
static bool init_modules_info(void)
{
  json_t* j_tmp;
  struct dirent **namelist;
  int i;
  int cnt;
  char* filename;
  char* full_filename;
  const char* dir;
  struct stat file_stat;
  int ret;
  char* timestamp;


  dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_module"));
  if(dir == NULL) {
    slog(LOG_ERR, "Could not get lib directory info.");
    return false;
  }

  // get directory info.
  cnt = scandir(dir, &namelist, NULL, versionsort);
  if(cnt < 0) {
    if(errno == ENOENT) {
      // just empty directory
      return true;
    }
    slog(LOG_ERR, "Could not get directory info. err[%d:%s]", errno, strerror(errno));
    return false;
  }

  // get file info
  timestamp = get_utc_timestamp();
  for(i = 0; i < cnt; i++) {
    // get stat
    asprintf(&full_filename, "%s/%s", dir, namelist[i]->d_name);
    ret = stat(full_filename, &file_stat);
    sfree(full_filename);
    if(ret != 0) {
      slog(LOG_ERR, "Could not get file info. filename[%s], err[%d:%s]", full_filename, errno, strerror(errno));
      free(namelist[i]);
      continue;
    }

    ret = strcmp(namelist[i]->d_name, ".");
    if(ret == 0) {
      free(namelist[i]);
      continue;
    }
    ret = strcmp(namelist[i]->d_name, "..");
    if(ret == 0) {
      free(namelist[i]);
      continue;
    }

    // strip extension
    filename = strip_ext(namelist[i]->d_name);
    if(filename == NULL) {
      slog(LOG_ERR, "Could not strip file info.");
      free(namelist[i]);
      continue;
    }

    // create data
    j_tmp = json_pack("{s:s, s:i, s:s, s:s}",
        "name", filename,
        "size", file_stat.st_size,
        "load", "unknown",

        "tm_update", timestamp
        );
    sfree(filename);

    // insert
    create_core_module(j_tmp);
    json_decref(j_tmp);

    free(namelist[i]);
  }
  free(namelist);
  sfree(timestamp);

  return true;
}


