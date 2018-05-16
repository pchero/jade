/*
 * sip_handler.c
 *
 *  Created on: Dec 12, 2017
 *      Author: pchero
 */

#include <stdio.h>
#include <jansson.h>
#include <publication_handler.h>
#include <string.h>

#include "slog.h"
#include "ami_handler.h"
#include "conf_handler.h"
#include "utils.h"
#include "conf_handler.h"

#include "http_handler.h"
#include "sip_handler.h"
#include "resource_handler.h"

#define DEF_SIP_CONFNAME  "sip.conf"

#define DEF_DB_TABLE_SIP_PEER           "sip_peer"
#define DEF_DB_TABLE_SIP_PEERACCOUNT    "sip_peeraccount"
#define DEF_DB_TABLE_SIP_REGISTRY       "sip_registry"


static bool init_sip_database(void);
static bool init_sip_database_peer(void);
static bool init_sip_database_peeraccount(void);
static bool init_sip_database_registry(void);

static bool init_sip_info(void);
static bool init_sip_info_registry(void);
static bool init_sip_info_peer(void);
static bool init_sip_info_peeraccount(void);

static bool term_sip_database(void);


bool sip_init_handler(void)
{
  int ret;

  slog(LOG_DEBUG, "Fired init_sip_handler.");

  // init database
  ret = init_sip_database();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip database.");
    return false;
  }

  // init info
  ret = init_sip_info();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip info.");
    return false;
  }

  return true;
}

bool sip_term_handler(void)
{
  int ret;

  ret = term_sip_database();
  if(ret == false) {
    slog(LOG_ERR, "Could not clear sip.");
    return false;
  }

  return true;
}

bool sip_reload_handler(void)
{
  int ret;

  ret = sip_term_handler();
  if(ret == false) {
    return false;
  }

  ret = sip_init_handler();
  if(ret == false) {
    return false;
  }

  return true;
}

static bool init_sip_database_peer(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_SIP_PEER ";";
  create_table =
      "create table " DEF_DB_TABLE_SIP_PEER " ("

      // identity
      "   peer         varchar(255)    not null,"
      "   status       varchar(255),"
      "   address      varchar(255),"     // ip_address:port

      // peer info
      "   channel_type     varchar(255),"   // channel type(SIP, ...)
      "   chan_object_type varchar(255),"   // (peer, ...)
      "   monitor_status   varchar(255),"

      "   dynamic          varchar(255),"   // dynamic(yes, ...)
      "   auto_force_port  varchar(255),"   //
      "   force_port       varchar(255),"   //
      "   auto_comedia     varchar(255),"   //
      "   comedia          varchar(255),"   //
      "   video_support    varchar(255),"   //
      "   text_support     varchar(255),"   //

      "   acl              varchar(255),"   //
      "   realtime_device  varchar(255),"   //
      "   description      varchar(255),"   //

      // timestamp. UTC."
      "   tm_update        datetime(6),"   // update time."

      "   primary key(peer)"

      ");";

  // execute
  ret = resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_peer database.");
    return false;
  }

  return true;
}

/**
 * Initiate sip_peeraccount database.
 * @return true:success, false:failed
 */
static bool init_sip_database_peeraccount(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_SIP_PEERACCOUNT ";";
  create_table =
      "create table " DEF_DB_TABLE_SIP_PEERACCOUNT " ("

      // identity
      "   peer         varchar(255)    not null,"
      "   secret       varchar(255),"

      // info
      "   host       varchar(255),"
      "   context    varchar(255),"
      "   type       varchar(255),"

      // timestamp. UTC."
      "   tm_update        datetime(6),"   // update time."

      "   primary key(peer)"

      ");";

  // execute
  ret = resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_peeraccount database.");
    return false;
  }

  return true;
}

/**
 * Initiate sip_registry database.
 * @return true:success, false:failed
 */
static bool init_sip_database_registry(void)
{
  int ret;
  const char* drop_table;
  const char* create_table;

  drop_table = "drop table if exists " DEF_DB_TABLE_SIP_REGISTRY ";";
  create_table =
      "create table " DEF_DB_TABLE_SIP_REGISTRY "("

      // identity
      "   account     varchar(255),"
      "   username    varchar(255),"
      "   host        varchar(255),"
      "   port        int,"

      // info
      "   domain            varchar(255),"
      "   domain_port       int,"
      "   refresh           int,"
      "   state             text,"
      "   registration_time int,"

      // timestamp. UTC."
      "   tm_update         datetime(6),"   // update time."

      "   primary key(account)"

      ");";

  // execute
  ret = resource_exec_mem_sql(drop_table);
  ret = resource_exec_mem_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_registry database.");
    return false;
  }

  return true;
}


/**
 * Initiate sip database.
 * @return
 */
static bool init_sip_database(void)
{
  int ret;

  ret = init_sip_database_peer();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_peer database.");
    return false;
  }

  ret = init_sip_database_peeraccount();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_peeraccount database.");
    return false;
  }

  ret = init_sip_database_registry();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_registry database.");
    return false;
  }

  return true;
}

/**
 * GET ^/sip/peers/(*) request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_peers_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_sip_peers_detail.");

  // get detail
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_NOTICE, "Could not get id info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get peer info.
  j_tmp = sip_get_peer_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get peer info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/sip/peers request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_peers(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_sip_peers.");

  j_tmp = sip_get_peers_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/sip/registries/(*) request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_registries_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_sip_registries_detail.");

  // get detail
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_NOTICE, "Could not get id info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info.
  j_tmp = sip_get_registry_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get registry info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/sip/registries request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_registries(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired htp_get_sip_registries.");

  j_tmp = sip_get_registries_all();
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/sip/config request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_config(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_sip_config.");

  // get info
  j_tmp = conf_get_ast_current_config_info_text(DEF_SIP_CONFNAME);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip conf.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * PUT ^/sip/config request handler.
 * @param req
 * @param data
 */
void sip_htp_put_sip_config(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* req_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_sip_config.");

  // get data
  req_data = http_get_text_from_request_data(req);
  if(req_data == NULL) {
    slog(LOG_ERR, "Could not get data.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update config
  ret = conf_update_ast_current_config_info_text(DEF_SIP_CONFNAME, req_data);
  sfree(req_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update sip config info.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/sip/configs request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_configs(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_sip_configs.");

  // get info
  j_tmp = conf_get_ast_backup_configs_text_all(DEF_SIP_CONFNAME);
  if(j_tmp == NULL) {
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/sip/configs/(.*) request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_configs_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_sip_configs_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get config info
  j_tmp = conf_get_ast_backup_config_info_text_valid(detail, DEF_SIP_CONFNAME);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find config info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * DELETE ^/sip/configs/(.*) request handler.
 * @param req
 * @param data
 */
void sip_htp_delete_sip_configs_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_sip_configs_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // remove it
  ret = conf_remove_ast_backup_config_info_valid(detail, DEF_SIP_CONFNAME);
  sfree(detail);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete config file.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: GET ^/sip/settings$
 * @param req
 * @param data
 */
void sip_htp_get_sip_settings(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_sip_settings.");

  // get info
  j_tmp = conf_get_ast_sections_all(DEF_SIP_CONFNAME);

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/sip/settings$
 * @param req
 * @param data
 */
void sip_htp_post_sip_settings(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  json_t* j_setting;
  int ret;
  const char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_sip_settings.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get data from request.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_ERR, "Could not get setting name.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_setting = json_object_get(j_data, "setting");
  if(j_setting == NULL) {
    slog(LOG_ERR, "Could not get setting.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  ret = conf_create_ast_section(DEF_SIP_CONFNAME, name, j_setting);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create sip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * GET ^/sip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void sip_htp_get_sip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_sip_settings_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get setting
  j_tmp = conf_get_ast_section(DEF_SIP_CONFNAME, detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * PUT ^/sip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void sip_htp_put_sip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_sip_settings_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    sfree(detail);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update setting
  ret = conf_update_ast_section(DEF_SIP_CONFNAME, detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update sip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * DELETE ^/sip/settings/(.*) request handler.
 * @param req
 * @param data
 */
void htp_delete_sip_settings_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_sip_settings_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete setting
  ret = conf_delete_ast_section(DEF_SIP_CONFNAME, detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not remove sip setting.");
    http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = http_create_default_result(EVHTP_RES_OK);

  // response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * Initiate all sip info.
 * @return
 */
static bool init_sip_info(void)
{
  int ret;

  ret = init_sip_info_peer();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_peer info.");
    return false;
  }

  ret = init_sip_info_registry();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_registry info.");
    return false;
  }

  ret = init_sip_info_peeraccount();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_peeraccount info.");
    return false;
  }


  return true;
}

/**
 * Initiate sip registry info.
 * @return
 */
static bool init_sip_info_registry(void)
{
  json_t* j_tmp;
  int ret;

  // registry
  j_tmp = json_pack("{s:s}",
      "Action", "SIPshowregistry"
      );
  ret = ami_send_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "SIPshowregistry");
    return false;
  }

  return true;
}

/**
 * Initiate sip peer info.
 * @return
 */
static bool init_sip_info_peer(void)
{
  json_t* j_tmp;
  int ret;

  // sip peers
  j_tmp = json_pack("{s:s}",
      "Action", "SipPeers"
      );
  ret = ami_send_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "SipPeers");
    return false;
  }

  return true;
}

/**
 * Init sip peeraccount info.
 * @return
 */
static bool init_sip_info_peeraccount(void)
{
  int ret;
  json_t* j_conf;
  json_t* j_tmp;
  json_t* j_info;
  char* timestamp;
  const char* key;

  j_conf = conf_get_ast_current_config_info(DEF_SIP_CONFNAME);
  if(j_conf == NULL) {
    slog(LOG_ERR, "Could not load sip config info.");
    return false;
  }

  timestamp = utils_get_utc_timestamp();
  json_object_foreach(j_conf, key, j_tmp) {
    if(key == NULL) {
      continue;
    }

    // check
    if((strcmp(key, "general") == 0) || (strcmp(key, "authentication") == 0)) {
      continue;
    }

    // create peeraccount
    j_info = json_pack("{"
        "s:s, s:s,"
        "s:s, s:s, s:s,"
        "s:s "
        "}",

        "peer",     key,
        "secret",   json_string_value(json_object_get(j_tmp, "secret"))? : "",

        "type",     json_string_value(json_object_get(j_tmp, "type"))? : "",
        "host",     json_string_value(json_object_get(j_tmp, "host"))? : "",
        "context",  json_string_value(json_object_get(j_tmp, "context"))? : "",

        "tm_update", timestamp
        );

    ret = sip_create_peeraccount_info(j_info);
    json_decref(j_info);
    if(ret == false) {
      slog(LOG_ERR, "Could not create peeraccount info. peer[%s]", key);
      json_decref(j_conf);
      sfree(timestamp);
      return false;
    }
  }

  json_decref(j_conf);
  sfree(timestamp);
  return true;
}

/**
 * Create sip peeraccount info.
 * @param j_data
 * @return
 */
bool sip_create_peeraccount_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_sip_peeraccount_info.");

  // insert peeraccount info
  ret = resource_insert_mem_item(DEF_DB_TABLE_SIP_PEERACCOUNT, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert sip peeraccount.");
    return false;
  }

  return true;
}

/**
 * Get given peeraccount's detail info.
 * @param peer
 * @return
 */
json_t* sip_get_peeraccount_info(const char* peer)
{
  json_t* j_res;

  if(peer == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_sip_peeraccount_info.");

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_SIP_PEERACCOUNT, "peer", peer);

  return j_res;
}

/**
 * Get given peer's detail info.
 * @return
 */
json_t* sip_get_peer_info(const char* peer)
{
  json_t* j_res;

  if(peer == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_sip_peer_info.");

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_SIP_PEER, "peer", peer);

  return j_res;
}

/**
 * Get all peers array
 * @return
 */
json_t* sip_get_peers_all_peer(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_sip_peers_all_peer.");

  j_res = resource_get_mem_items(DEF_DB_TABLE_SIP_PEER, "peer");

  return j_res;
}

/**
 * Get all peers array
 * @return
 */
json_t* sip_get_peers_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_sip_peers_all.");

  j_res = resource_get_mem_items(DEF_DB_TABLE_SIP_PEER, "*");

  return j_res;
}

/**
 * Get all registry account array
 * @return
 */
json_t* sip_get_registries_all_account(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_sip_registries_all_account.");

  j_res = resource_get_mem_items(DEF_DB_TABLE_SIP_REGISTRY, "account");

  return j_res;
}

/**
 * Get corresponding registry info.
 * @return
 */
json_t* sip_get_registry_info(const char* account)
{
  json_t* j_res;

  if(account == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_sip_registry_info. account[%s]", account);

  j_res = resource_get_mem_detail_item_key_string(DEF_DB_TABLE_SIP_REGISTRY, "account", account);

  return j_res;
}

/**
 * Get all registries array
 * @return
 */
json_t* sip_get_registries_all(void)
{
  json_t* j_res;

  j_res = resource_get_mem_items(DEF_DB_TABLE_SIP_REGISTRY, "*");
  return j_res;
}

/**
 * Create sip peer info.
 * @param j_data
 * @return
 */
bool sip_create_peer_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_sip_peer_info.");

  // insert peer info
  ret = resource_insert_mem_item(DEF_DB_TABLE_SIP_PEER, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert sip peer..");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "peer"));
  j_tmp = sip_get_peer_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip_peer info. peer[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_sip_peer(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update sip peer info.
 * @param j_data
 * @return
 */
bool sip_update_peer_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_sip_peer_info.");

  ret = resource_update_mem_item(DEF_DB_TABLE_SIP_PEER, "peer", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update sip peer info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "peer"));
  j_tmp = sip_get_peer_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip_peer info. peer[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_sip_peer(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete sip peer info.
 * @return
 */
bool sip_delete_peer_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_sip_peer_info. peer[%s]", key);

  // get info
  j_tmp = sip_get_peer_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip_peer info. peer[%s]", key);
    return false;
  }

  ret = resource_delete_mem_items_string(DEF_DB_TABLE_SIP_PEER, "peer", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete sip peer info. name[%s]", key);
    json_decref(j_tmp);
    return false;
  }

  // publish
  // publish event
  ret = publication_publish_event_sip_peer(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Create sip registry info.
 * @param j_data
 * @return
 */
bool sip_create_registry_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_sip_registry_info.");

  // insert queue info
  ret = resource_insert_mem_item(DEF_DB_TABLE_SIP_REGISTRY, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert sip registry.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "account"));
  j_tmp = sip_get_registry_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip_registry info. account[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_sip_registry(DEF_PUB_TYPE_CREATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Update sip registry info.
 * @param j_data
 * @return
 */
bool sip_update_registry_info(const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_sip_registry_info.");

  ret = resource_update_mem_item(DEF_DB_TABLE_SIP_REGISTRY, "account", j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update sip registry info.");
    return false;
  }

  // publish
  // get info
  tmp_const = json_string_value(json_object_get(j_data, "account"));
  j_tmp = sip_get_registry_info(tmp_const);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip_registry info. account[%s]", tmp_const);
    return false;
  }

  // publish event
  ret = publication_publish_event_sip_registry(DEF_PUB_TYPE_UPDATE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * delete sip registry info.
 * @return
 */
bool sip_delete_registry_info(const char* key)
{
  int ret;
  json_t* j_tmp;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_sip_registry_info. account[%s]", key);

  // get info
  j_tmp = sip_get_registry_info(key);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get sip_registry info. account[%s]", key);
    return false;
  }

  ret = resource_delete_mem_items_string(DEF_DB_TABLE_SIP_REGISTRY, "account", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete sip registry info. account[%s]", key);
    return false;
  }

  // publish
  // publish event
  ret = publication_publish_event_sip_registry(DEF_PUB_TYPE_DELETE, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Clear all sip resources.
 * @return
 */
static bool term_sip_database(void)
{
  int ret;

  // peer
  ret = resource_clear_mem_table(DEF_DB_TABLE_SIP_PEER);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear sip table. table[%s]", DEF_DB_TABLE_SIP_PEER);
    return false;
  }

  // peeraccount
  ret = resource_clear_mem_table(DEF_DB_TABLE_SIP_PEERACCOUNT);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear sip table. table[%s]", DEF_DB_TABLE_SIP_PEERACCOUNT);
    return false;
  }

  // registry
  ret = resource_clear_mem_table(DEF_DB_TABLE_SIP_REGISTRY);
  if(ret == false) {
    slog(LOG_ERR, "Could not clear table. table[%s]", DEF_DB_TABLE_SIP_REGISTRY);
    return false;
  }

  return true;
}




