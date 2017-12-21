/*
 * config.c
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdbool.h>
#include <jansson.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "common.h"
#include "slog.h"
#include "minIni.h"
#include "utils.h"

#include "config.h"

#define DEF_JADE_LIB_DIR  "/var/lib/jade"

#define DEF_AST_CONF_BACKUP_DIR "confs"


#define DEF_CONF_FILENAME "./jade.conf"

#define DEF_GENERAL_AMI_SERV_ADDR "127.0.0.1"
#define DEF_GENERAL_AMI_SERV_PORT "5038"
#define DEF_GENERAL_HTTP_ADDR "0.0.0.0"
#define DEF_GENERAL_HTTP_PORT "8081"
#define DEF_GENERAL_ZMQ_ADDR_PUBLISH "tcp://*:8082"   // zmq address for publish
#define DEF_GENERAL_LOGLEVEL  "5"
#define DEF_GENERAL_DATABASE_NAME ":memory:"
#define DEF_GENERAL_EVENT_TIME_FAST "100000"
#define DEF_GENERAL_EVENT_TIME_SLOW "3000000"
#define DEF_GENERAL_DIR_CONF	"/etc/asterisk"
#define DEF_GENERAL_DIR_MODULE   "/usr/lib/asterisk/modules"

#define DEF_VOICEMAIL_DIRECTORY "/var/spool/asterisk/voicemail"
#define DEF_VOICEMAIL_CONFNAME "voicemail.conf"

#define DEF_OB_DIALING_RESULT_FILENAME  "./outbound_result.json"
#define DEF_OB_DIALING_TIMEOUT          "30"
#define DEF_OB_DATABASE_NAME  "./outbound_database.db"

extern app* g_app;
static char g_config_filename[1024] = "";


static bool load_config(void);
static bool write_config(void);

static int write_ast_config_info(const char* filename, json_t* j_conf);

static char* get_ast_backup_conf_dir(void);
static json_t* get_ast_backup_filenames(const char* filename);
static int ast_conf_handler(const mTCHAR *section, const mTCHAR *key, const mTCHAR *value, void *data);
static int backup_ast_config_info(const char* filename);
static bool create_lib_dirs(void);
static json_t* get_ast_config_info(const char* filename);



/**
 * Initiate configuration
 * @return
 */
bool init_config(void)
{
  int ret;

  if((g_config_filename == NULL) || (strlen(g_config_filename) == 0)) {
    snprintf(g_config_filename, sizeof(g_config_filename), DEF_CONF_FILENAME);
  }

  ret = load_config();
  if(ret == false) {
    return false;
  }

  ret = create_lib_dirs();
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Update config filename
 * @param filename
 * @return
 */
bool update_config_filename(const char* filename)
{
  if(filename == NULL) {
    printf("Wrong input parameter.");
    return false;
  }

  snprintf(g_config_filename, sizeof(g_config_filename), "%s", filename);
  printf("Updated config filename. config_filename[%s]", g_config_filename);

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
 * Load configuration file and update
 * @return
 */
static bool load_config(void)
{
  const char* tmp_const;
  int ret;
  json_t* j_conf_def;
  json_t* j_conf;
  const char* key;
  json_t* j_tmp;

  slog(LOG_INFO, "Load configuration file. filename[%s]", g_config_filename);

  // create default conf
  j_conf_def = json_pack("{"
      "s:{"
      	"s:s, s:s, s:s, s:s, s:s, "
      	"s:s, "
      	"s:s, s:s, "
        "s:s, "
      	"s:s, s:s, "
      	"s:s, s:s "
			"},"	// general
      "s:{s:s, s:s}, "	// voicemail
      "s:{s:s, s:s, s:s}"	// ob
      "}",
      "general",
        "ami_serv_addr",    DEF_GENERAL_AMI_SERV_ADDR,
        "ami_serv_port",    DEF_GENERAL_AMI_SERV_PORT,
        "ami_username",     "",
        "ami_password",     "",
        "loglevel",         DEF_GENERAL_LOGLEVEL,

        "database_name",    DEF_GENERAL_DATABASE_NAME,

        "http_addr",        DEF_GENERAL_HTTP_ADDR,
        "http_port",        DEF_GENERAL_HTTP_PORT,

        "zmq_addr_pub",     DEF_GENERAL_ZMQ_ADDR_PUBLISH,

        "event_time_fast",  DEF_GENERAL_EVENT_TIME_FAST,
        "event_time_slow",  DEF_GENERAL_EVENT_TIME_SLOW,

				"directory_conf",		DEF_GENERAL_DIR_CONF,
				"directory_module", DEF_GENERAL_DIR_MODULE,

      "voicemail",
        "dicretory",        DEF_VOICEMAIL_DIRECTORY,
				"conf_name",				DEF_VOICEMAIL_CONFNAME,

      "ob",
        "dialing_result_filename",  DEF_OB_DIALING_RESULT_FILENAME,
        "dialing_timeout",          DEF_OB_DIALING_TIMEOUT,
        "database_name",            DEF_OB_DATABASE_NAME
      );
  if(j_conf_def == NULL) {
    printf("Could not create default config.\n");
    return false;
  }

  j_conf = json_load_file(g_config_filename, JSON_DECODE_ANY, NULL);

  // update conf
  json_object_foreach(j_conf_def, key, j_tmp) {
  	json_object_update(j_tmp, json_object_get(j_conf, key));
  }
  json_decref(j_conf);

  if(g_app->j_conf != NULL) {
    json_decref(g_app->j_conf);
  }

  g_app->j_conf = j_conf_def;
  write_config();

  // update log level
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "loglevel"));
  ret = atoi(tmp_const);
  update_log_level(ret);

  slog(LOG_DEBUG, "load_config end.");
  return true;
}

/**
 * Write current config optin to the file.
 * @return
 */
static bool write_config(void)
{
  if(g_app->j_conf == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired write_config.");

  json_dump_file(g_app->j_conf, g_config_filename, JSON_INDENT(4));

  return true;
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
  slog(LOG_DEBUG, "Fired get_ast_backup_config_info.");

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
 * @param filename
 * @return
 */
char* get_ast_current_config_info_raw(const char* filename)
{

  // todo: not implemented yet.
  slog(LOG_ERR, "Not implemented yet.");
  return NULL;
}


/**
 * Update asterisk configuration file info.
 * @param j_conf
 * @param filename
 * @return
 */
int update_ast_current_config_info(const char* filename, json_t* j_conf)
{
  int ret;

  if((j_conf == NULL) || (filename == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // backup current config
  ret = backup_ast_config_info(filename);
  if(ret == false) {
    slog(LOG_ERR, "Could not backup current config file. filename[%s]", filename);
    return false;
  }

  ret = write_ast_config_info(filename, j_conf);
  if(ret == false) {
    slog(LOG_ERR, "Could not update config. filename[%s]", filename);
    return false;
  }

  return true;
}

/**
 * Update asterisk configuration file.
 * @param j_conf
 * @param filename
 * @return
 */
int update_ast_config_info_raw(const char* filename, const char* data)
{
  if((filename == NULL) || (data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_ERR, "Not implemented yet");

  return false;
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
  const char* conf_dir;
  char* cmd;
  int ret;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired backup_ast_config_info. filename[%s]", filename);

  timestamp = get_utc_timestamp();
  backup_dir = get_ast_backup_conf_dir();
  conf_dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_conf"));

  // create full target filename
  asprintf(&target, "%s/%s.%s", backup_dir, filename, timestamp);
  sfree(backup_dir);
  sfree(timestamp);

  // create cmd for copy config file to backdup dir
  asprintf(&cmd, "cp -r %s/%s %s", conf_dir, filename, target);
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

static int write_ast_config_info(const char* filename, json_t* j_conf)
{
  const char* section;
  const char* dir;
  const char* key;
  char* target;
  json_t* j_item;
  json_t* j_val;
  json_t* j_val_tmp;
  FILE *f;
  int ret;
  int idx;

  if((j_conf == NULL) || (filename == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired write_ast_config_info. filename[%s]", filename);

  // create target
  dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_conf"));
  asprintf(&target, "%s/%s", dir, filename);

  f = fopen(target, "w+");
  if(f == NULL) {
    slog(LOG_ERR, "Could not open the conf file. filename[%s], err[%d:%s]", target, errno, strerror(errno));
    return false;
  }
  sfree(target);

  json_object_foreach(j_conf, section, j_item) {
    fprintf(f, "[%s]\n", section);
    json_object_foreach(j_item, key, j_val) {

      // check is array
      ret = json_is_array(j_val);
      if(ret == true) {
        json_array_foreach(j_val, idx, j_val_tmp) {
          fprintf(f, "%s=%s\n", key, json_string_value(j_val_tmp));
        }
      }
      else {
        fprintf(f, "%s=%s\n", key, json_string_value(j_val));
      }
    }
  }
  fclose(f);

  return true;
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

json_t* get_ast_backup_configs_info_all(const char* filename)
{
  json_t* j_res;
  json_t* j_list;
  json_t* j_tmp;
  json_t* j_conf;
  json_t* j_conf_tmp;
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

    j_conf_tmp = json_object();
    json_object_set_new(j_conf_tmp, tmp_const, j_conf);

    // add to array
    json_array_append_new(j_res, j_conf_tmp);
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

/**
 * Update asterisk configuration file content.
 * @param j_conf
 * @param filename
 * @return
 */
int update_ast_current_config_content(const char* filename, const char* section, const char* key, const char* val)
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

  // get section, if not, create new.
  j_section = json_object_get(j_conf, section);
  if(j_section == NULL) {
    j_section = json_object();
    json_object_set_new(j_conf, section, j_section);
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
