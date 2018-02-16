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

#include "common.h"
#include "slog.h"
#include "minIni.h"
#include "utils.h"

#include "config.h"

#define DEF_CONF_FILENAME "./jade.conf"

#define DEF_GENERAL_AST_SERV_ADDR "127.0.0.1"
#define DEF_GENERAL_AMI_SERV_ADDR "127.0.0.1"
#define DEF_GENERAL_AMI_SERV_PORT "5038"
#define DEF_GENERAL_HTTPS_ADDR "0.0.0.0"
#define DEF_GENERAL_HTTPS_PORT "8081"
#define DEF_GENERAL_HTTPS_PEMFILE "./jade.pem"
#define DEF_GENERAL_ZMQ_ADDR_PUBLISH "tcp://*:8082"   // zmq address for publish
#define DEF_GENERAL_WEBSOCK_ADDR "0.0.0.0"
#define DEF_GENERAL_WEBSOCK_PORT "8083"
#define DEF_GENERAL_LOGLEVEL  "5"
#define DEF_GENERAL_DATABASE_NAME_AST   ":memory:"
#define DEF_GENERAL_DATABASE_NAME_JADE  "./jade_database.db"
#define DEF_GENERAL_EVENT_TIME_FAST "100000"
#define DEF_GENERAL_EVENT_TIME_SLOW "3000000"
#define DEF_GENERAL_DIR_CONF	"/etc/asterisk"
#define DEF_GENERAL_DIR_MODULE   "/usr/lib/asterisk/modules"

#define DEF_VOICEMAIL_DIRECTORY "/var/spool/asterisk/voicemail"

#define DEF_OB_DIALING_RESULT_FILENAME  "./outbound_result.json"
#define DEF_OB_DIALING_TIMEOUT          "30"
#define DEF_OB_DATABASE_NAME  "./outbound_database.db"

extern app* g_app;
static char g_config_filename[1024] = "";


static bool load_config(void);
static bool write_config(void);





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
      	"s:s, s:s, s:s, s:s, s:s, s:s, "
      	"s:s, s:s, "
      	"s:s, s:s, s:s, "
        "s:s, s:s, s:s, "
      	"s:s, s:s, "
      	"s:s, s:s "
			"},"	// general
      "s:{s:s}, "	// voicemail
      "s:{s:s, s:s, s:s}"	// ob
      "}",
      "general",
        "ast_serv_addr",    DEF_GENERAL_AST_SERV_ADDR,
        "ami_serv_addr",    DEF_GENERAL_AMI_SERV_ADDR,
        "ami_serv_port",    DEF_GENERAL_AMI_SERV_PORT,
        "ami_username",     "",
        "ami_password",     "",
        "loglevel",         DEF_GENERAL_LOGLEVEL,

        "database_name_ast",    DEF_GENERAL_DATABASE_NAME_AST,
        "database_name_jade",   DEF_GENERAL_DATABASE_NAME_JADE,

        "https_addr",       DEF_GENERAL_HTTPS_ADDR,
        "https_port",       DEF_GENERAL_HTTPS_PORT,
        "https_pemfile",    DEF_GENERAL_HTTPS_PEMFILE,

        "zmq_addr_pub",     DEF_GENERAL_ZMQ_ADDR_PUBLISH,
        "websock_addr",     DEF_GENERAL_WEBSOCK_ADDR,
        "websock_port",     DEF_GENERAL_WEBSOCK_PORT,

        "event_time_fast",  DEF_GENERAL_EVENT_TIME_FAST,
        "event_time_slow",  DEF_GENERAL_EVENT_TIME_SLOW,

				"directory_conf",		DEF_GENERAL_DIR_CONF,
				"directory_module", DEF_GENERAL_DIR_MODULE,

      "voicemail",
        "dicretory",        DEF_VOICEMAIL_DIRECTORY,

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





