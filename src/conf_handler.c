/*
 * configfile_handler.c
 *
 *  Created on: Jan 21, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include "slog.h"
#include "common.h"
#include "utils.h"
#include "minIni.h"
#include "conf_handler.h"

extern app* g_app;

#define DEF_JADE_LIB_DIR          "/var/lib/jade"
#define DEF_AST_CONF_BACKUP_DIR   "confs"

#define MAX_CONF_BUF    1048576   // 10 Mb(1024 * 1024)

static int write_ast_config_info(const char* filename, json_t* j_conf);
static bool write_ast_config_info_raw(const char* filename, const char* data);

static char* get_ast_backup_conf_dir(void);
static json_t* get_ast_backup_filenames(const char* filename);
static int ast_conf_handler(const mTCHAR *section, const mTCHAR *key, const mTCHAR *value, void *data);
static int backup_ast_config_info(const char* filename);
static bool create_lib_dirs(void);

static json_t* get_ast_config_info(const char* filename);
static char* get_ast_config_info_raw(const char* filename);

bool init_conf_handler(void)
{
  int ret;

  ret = create_lib_dirs();
  if(ret == false) {
    slog(LOG_ERR, "Could not create lib directory.");
    return false;
  }

  return true;
}

static int write_ast_config_info(const char* filename, json_t* j_conf)
{
  const char* section;
  const char* key;
  json_t* j_item;
  json_t* j_val;
  json_t* j_val_tmp;
  FILE *fp;
  int ret;
  int idx;

  if((j_conf == NULL) || (filename == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired write_ast_config_info. filename[%s]", filename);

  fp = fopen(filename, "w+");
  if(fp == NULL) {
    slog(LOG_ERR, "Could not open the conf file. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return false;
  }

  // write config
  json_object_foreach(j_conf, section, j_item) {
    slog(LOG_DEBUG, "Writing section info. section[%s]", section);
    fprintf(fp, "[%s]\n", section);
    json_object_foreach(j_item, key, j_val) {

      // check is array
      ret = json_is_array(j_val);
      if(ret == true) {
        json_array_foreach(j_val, idx, j_val_tmp) {
          fprintf(fp, "%s=%s\n", key, json_string_value(j_val_tmp));
          slog(LOG_DEBUG, "Writing item info. key[%s] value[%s]", key, json_string_value(j_val_tmp));
        }
      }
      else {
        fprintf(fp, "%s=%s\n", key, json_string_value(j_val));
        slog(LOG_DEBUG, "Writing item info. key[%s] value[%s]", key, json_string_value(j_val));
      }
    }
  }
  fclose(fp);

  return true;
}

static bool write_ast_config_info_raw(const char* filename, const char* data)
{
  FILE *fp;
  int ret;

  if((filename == NULL) || (data == NULL) || (strlen(data) == 0)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired write_ast_config_info_raw. filename[%s]", filename);

  fp = fopen(filename, "w+");
  if(fp == NULL) {
    slog(LOG_ERR, "Could not open the conf file. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return false;
  }

  ret = fwrite(data, 1, strlen(data), fp);
  fclose(fp);
  if(ret == 0) {
    slog(LOG_ERR, "Could not write config info.");
    return false;
  }

  return true;
}

/**
 * Return asterisk backup dir name
 * @return
 */
static char* get_ast_backup_conf_dir(void)
{
  char* res;

  asprintf(&res, "%s/%s", DEF_JADE_LIB_DIR, DEF_AST_CONF_BACKUP_DIR);
  return res;
}


/**
 * Get backup filename lists of given filename
 * @param filename
 * @return
 */
static json_t* get_ast_backup_filenames(const char* filename)
{
  json_t* j_res;
  struct dirent **namelist;
  int i;
  int cnt;
  char* tmp;
  char* dir;

  if(filename == NULL) {
    return NULL;
  }

  j_res = json_array();
  dir = get_ast_backup_conf_dir();

  // get directory info.
  cnt = scandir(dir, &namelist, NULL, versionsort);
  sfree(dir);
  if(cnt < 0) {
    if(errno == ENOENT) {
      return j_res;
    }

    slog(LOG_ERR, "Could not get directory info. err[%d:%s]", errno, strerror(errno));
    json_decref(j_res);
    return NULL;
  }

  // get file info
  for(i = 0; i < cnt; i++) {
    tmp = strstr(namelist[i]->d_name, filename);
    if(tmp != NULL) {
      json_array_append_new(j_res, json_string(namelist[i]->d_name));
    }
    free(namelist[i]);
  }
  free(namelist);

  return j_res;
}

static int ast_conf_handler(const mTCHAR *section, const mTCHAR *key, const mTCHAR *value, void *data)
{
  json_t* j_res;
  json_t* j_sec;
  json_t* j_item;
  json_t* j_arr;
  int ret;

  j_res = (json_t*)data;

  // check section.
  // if not exist, create new one.
  j_sec = json_object_get(j_res, section);
  if(j_sec == NULL) {
    json_object_set_new(j_res, section, json_object());
    j_sec = json_object_get(j_res, section);
  }

  if(key == NULL) {
    // there is no key. section only
    return 1;
  }


  // check item existence.
  // if there's key already, add it as a array.
  j_item = json_object_get(j_sec, key);
  if(j_item != NULL) {
    ret = json_is_array(j_item);
    if(ret == true) {
      // if already exist, append it
      json_array_append_new(j_item, json_string(value));
    }
    else {
      // if not exist,
      // substitute to array.
      j_arr = json_array();
      json_array_append_new(j_arr, json_string(value));
      json_array_append(j_arr, j_item);
      json_object_set_new(j_sec, key, j_arr);
    }
  }
  else {
    // add key/value
    json_object_set_new(j_sec, key, json_string(value));
  }

  return 1;
}

/**
 * backup the given asterisk configuration file.
 * @param filename
 * @return
 */
static int backup_ast_config_info(const char* filename)
{
  char* target;
  char* timestamp;
  char* backup_dir;
  char* cmd;
  char* tmp;
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired backup_ast_config_info. filename[%s]", filename);

  timestamp = get_utc_timestamp();
  backup_dir = get_ast_backup_conf_dir();

  // create full target filename
  tmp = basename(filename);
  asprintf(&target, "%s/%s.%s", backup_dir, tmp, timestamp);
  sfree(backup_dir);
  sfree(timestamp);

  // create cmd for copy config file to backdup dir
  asprintf(&cmd, "cp -r %s %s", filename, target);
  sfree(target);
  slog(LOG_DEBUG, "Copy config file. cmd[%s]", cmd);

  // execute
  ret = system(cmd);
  sfree(cmd);
  if(ret != EXIT_SUCCESS) {
    slog(LOG_ERR, "Could not backup the config file. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return false;
  }

  return true;
}

static bool create_lib_dirs(void)
{
  int ret;
  char* cmd;
  char* tmp;

  slog(LOG_DEBUG, "Fired create_lib_dirs.");

  asprintf(&tmp, "%s/%s", DEF_JADE_LIB_DIR, DEF_AST_CONF_BACKUP_DIR);

  asprintf(&cmd, "mkdir -p %s", tmp);
  sfree(tmp);

  ret = system(cmd);
  sfree(cmd);
  if(ret != EXIT_SUCCESS) {
    return false;
  }

  return true;
}

/**
 * Get current configuration info of given filename.
 * @param filename
 * @return
 */
static json_t* get_ast_config_info(const char* filename)
{
  json_t* j_res;
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_current_config_info. filename[%s]", filename);

  j_res = json_object();
  ret = ini_browse(ast_conf_handler, j_res, filename);
  if(ret == 0) {
    json_decref(j_res);
    slog(LOG_ERR, "Could not get asterisk config.");
    return NULL;
  }

  return j_res;
}

/**
 * Get config file in a raw format
 * @param filename
 * @return
 */
static char* get_ast_config_info_raw(const char* filename)
{
  FILE* fp;
  char buf[MAX_CONF_BUF];
  char* res;
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  fp = fopen(filename, "r");
  if(fp == NULL) {
    slog(LOG_ERR, "Could not open the conf file. filename[%s], err[%d:%s]", filename, errno, strerror(errno));
    return NULL;
  }

  ret = fread(buf, 1, MAX_CONF_BUF, fp);
  if(ret == 0) {
    fclose(fp);
    slog(LOG_ERR, "Could not read conf file. err[%d:%s]", errno, strerror(errno));
    return NULL;
  }
  buf[ret] = '\0';
  fclose(fp);

  res = strdup(buf);

  return res;
}

/**
 * Get current config info from given filename.
 * @param filename
 * @return
 */
json_t* get_ast_current_config_info(const char* filename)
{
  json_t* j_conf;
  const char* dir;
  char* target;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_current_config_info.");

  dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_conf"));
  asprintf(&target, "%s/%s", dir, filename);

  j_conf = get_ast_config_info(target);
  sfree(target);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config file info.");
    return NULL;
  }
  return j_conf;
}

/**
 * Get current given filename config info in a raw format.
 * @param filename
 * @return
 */
char* get_ast_current_config_info_raw(const char* filename)
{
  const char* dir;
  char* target;
  char* res;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_current_config_info_raw.");

  dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_conf"));
  asprintf(&target, "%s/%s", dir, filename);

  res = get_ast_config_info_raw(target);
  sfree(target);
  if(res == NULL) {
    slog(LOG_ERR, "Could not get config file info.");
    return NULL;
  }

  return res;
}


/**
 * Update asterisk configuration file info.
 * @param j_conf
 * @param filename
 * @return
 */
bool update_ast_current_config_info(const char* filename, json_t* j_conf)
{
  int ret;
  char* target;
  const char* conf_dir;

  if((j_conf == NULL) || (filename == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // create target
  conf_dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_conf"));
  if(conf_dir == NULL) {
    slog(LOG_ERR, "Could not get conf directory info.");
    return false;
  }
  asprintf(&target, "%s/%s", conf_dir, filename);

  // backup current config
  ret = backup_ast_config_info(target);
  if(ret == false) {
    slog(LOG_ERR, "Could not backup current config file. filename[%s]", filename);
    sfree(target);
    return false;
  }

  // overwrite data
  ret = write_ast_config_info(target, j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update config. filename[%s]", filename);
    sfree(target);
    return false;
  }

  sfree(target);

  return true;
}

/**
 * Update asterisk configuration file.
 * @param j_conf
 * @param filename
 * @return
 */
bool update_ast_config_info_raw(const char* filename, const char* data)
{
  const char* conf_dir;
  char* target;
  int ret;

  if((filename == NULL) || (data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_ERR, "Not implemented yet");

  conf_dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_conf"));
  if(conf_dir == NULL) {
    slog(LOG_ERR, "Could not get conf directory info.");
    return false;
  }
  asprintf(&target, "%s/%s", conf_dir, filename);

  // backup current config
  ret = backup_ast_config_info(target);
  if(ret == false) {
    slog(LOG_ERR, "Could not backup current config file. filename[%s]", filename);
    sfree(target);
    return false;
  }

  // overwrite data
  ret = write_ast_config_info_raw(target, data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update config. filename[%s]", filename);
    sfree(target);
    return false;
  }

  sfree(target);

  return false;
}



/**
 * Get config info from given filename.
 * @param filename
 * @return
 */
json_t* get_ast_backup_config_info(const char* filename)
{
  char* tmp;
  char* full_filename;
  json_t* j_conf;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ast_backup_config_info.");

  // create full filename
  tmp = get_ast_backup_conf_dir();
  asprintf(&full_filename, "%s/%s", tmp, filename);
  sfree(tmp);

  j_conf = get_ast_config_info(full_filename);
  sfree(full_filename);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config file info.");
    return NULL;
  }

  return j_conf;
}

/**
 * Get all configs info.
 * Add the "filename" item to all the configs.
 * @param filename
 * @return
 */
json_t* get_ast_backup_configs_info_all(const char* filename)
{
  json_t* j_res;
  json_t* j_list;
  json_t* j_tmp;
  json_t* j_conf;
  const char* tmp_const;
  int idx;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_list = get_ast_backup_filenames(filename);
  if(j_list == NULL) {
    slog(LOG_ERR, "Could not ast conf file list.");
    return NULL;
  }

  j_res = json_array();
  json_array_foreach(j_list, idx, j_tmp) {

    // get filename
    tmp_const = json_string_value(j_tmp);

    j_conf = get_ast_backup_config_info(tmp_const);
    if(j_conf == NULL) {
      slog(LOG_ERR, "Could not get config file info.");
      continue;
    }

    // set file name info
    json_object_set_new(j_conf, "filename", json_string(tmp_const));

    // add to array
    json_array_append_new(j_res, j_conf);
  }
  json_decref(j_list);

  return j_res;
}

/**
 * Remove config info of given filename.
 * @param filename
 * @return
 */
int remove_ast_backup_config_info(const char* filename)
{
  char* tmp;
  char* full_filename;
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired remove_ast_backup_config_info. filename[%s]", filename);

  // create full filename
  tmp = get_ast_backup_conf_dir();
  asprintf(&full_filename, "%s/%s", tmp, filename);
  sfree(tmp);

  ret = remove(full_filename);
  sfree(full_filename);
  if(ret != 0) {
    slog(LOG_ERR, "Could not remove file. err[%d:%s]", errno, strerror(errno));
    return false;
  }

  return true;
}



/**
 * Update asterisk configuration file content.
 * If not exists, return false
 * @param j_conf
 * @param filename
 * @return
 */
bool update_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val)
{
  int ret;
  json_t* j_conf;
  json_t* j_section;

  if((filename == NULL) || (section == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get config
  j_conf = get_ast_current_config_info(filename);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config info.");
    return false;
  }

  // get section, if not, return false.
  j_section = json_object_get(j_conf, section);
  if(j_section == NULL) {
    json_decref(j_conf);
    return false;
  }

  // update or insert
  json_object_set_new(j_section, key, json_string(val));

  // write conf
  ret = update_ast_current_config_info(filename, j_conf);
  json_decref(j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update current ast config info.");
    return false;
  }

  return true;
}

/**
 * Create asterisk configuration file content.
 * If already exist, return false.
 * @param j_conf
 * @param filename
 * @return
 */
bool create_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val)
{
  int ret;
  json_t* j_conf;
  json_t* j_section;
  json_t* j_tmp;

  if((filename == NULL) || (section == NULL) || (key == NULL) || (val == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get config
  j_conf = get_ast_current_config_info(filename);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config info.");
    return false;
  }

  // get section, if not, create new.
  j_section = json_object_get(j_conf, section);
  if(j_section == NULL) {
    j_section = json_object();
    json_object_set_new(j_conf, section, j_section);
  }

  // check existence
  j_tmp = json_object_get(j_section, key);
  if(j_tmp != NULL) {
    json_decref(j_conf);
    slog(LOG_NOTICE, "The content is already exist.");
    return false;
  }

  // insert
  json_object_set_new(j_section, key, json_string(val));

  // update conf
  ret = update_ast_current_config_info(filename, j_conf);
  json_decref(j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update current ast config info.");
    return false;
  }

  return true;
}

/**
 * Delete asterisk configuration file content.
 * If already exist, return false.
 * @param j_conf
 * @param filename
 * @return
 */
bool delete_ast_current_config_content(const char* filename, const char* section, const char* key)
{
  int ret;
  json_t* j_conf;

  if((filename == NULL) || (section == NULL) || (key == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get config
  j_conf = get_ast_current_config_info(filename);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config info.");
    return false;
  }

  // delete key
  json_object_del(json_object_get(j_conf, section), key);

  // update conf
  ret = update_ast_current_config_info(filename, j_conf);
  json_decref(j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update current ast config info.");
    return false;
  }

  return true;
}

/**
 * Create asterisk configuration file section with given data.
 * If already exist, return false.
 * @param j_conf
 * @param filename
 * @return
 */
bool create_ast_current_config_section_data(const char* filename, const char* section, json_t* j_data)
{
  int ret;
  json_t* j_conf;
  json_t* j_section;

  if((filename == NULL) || (section == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get config
  j_conf = get_ast_current_config_info(filename);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config info.");
    return false;
  }

  // get section, if exists return false
  j_section = json_object_get(j_conf, section);
  if(j_section != NULL) {
    slog(LOG_ERR, "Section is already exist.");
    json_decref(j_conf);
    return false;
  }

  // set data
  json_object_set(j_conf, section, j_data);

  // update conf
  ret = update_ast_current_config_info(filename, j_conf);
  json_decref(j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update current ast config info.");
    return false;
  }

  return true;
}

/**
 * Update asterisk configuration file section with given data.
 * If already exist, return false.
 * @param j_conf
 * @param filename
 * @return
 */
bool update_ast_current_config_section_data(const char* filename, const char* section, json_t* j_data)
{
  int ret;
  json_t* j_conf;
  json_t* j_section;

  if((filename == NULL) || (section == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ast_current_config_section_data. filename[%s]", filename);

  // get config
  j_conf = get_ast_current_config_info(filename);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config info.");
    return false;
  }

  // get section, if not exists return false
  j_section = json_object_get(j_conf, section);
  if(j_section == NULL) {
    slog(LOG_ERR, "Section is not exist.");
    json_decref(j_conf);
    return false;
  }

  // set data
  json_object_set(j_conf, section, j_data);

  // update conf
  ret = update_ast_current_config_info(filename, j_conf);
  json_decref(j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update current ast config info.");
    return false;
  }

  return true;
}

/**
 * Delete asterisk configuration file section.
 * If not exist, return true.
 * @param j_conf
 * @param filename
 * @return
 */
bool delete_ast_current_config_section(const char* filename, const char* section)
{
  int ret;
  json_t* j_conf;

  if((filename == NULL) || (section == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_ast_current_config_section. filename[%s], section[%s]", filename, section);

  // get config
  j_conf = get_ast_current_config_info(filename);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not get config info.");
    return false;
  }

  // delete section
  json_object_del(j_conf, section);

  // update conf
  ret = update_ast_current_config_info(filename, j_conf);
  json_decref(j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update current ast config info.");
    return false;
  }

  return true;
}
