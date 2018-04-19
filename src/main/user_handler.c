/*
 * user_handler.c
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */


#include <event.h>
#include <string.h>

#include "slog.h"
#include "common.h"
#include "utils.h"

#include "http_handler.h"
#include "resource_handler.h"
#include "sip_handler.h"
#include "pjsip_handler.h"

#include "user_handler.h"

extern app* g_app;

#define DEF_DB_TABLE_USER_CONTACT     "user_contact"
#define DEF_DB_TABLE_USER_USERINFO    "user_userinfo"
#define DEF_DB_TABLE_USER_GROUP       "user_group"
#define DEF_DB_TABLE_USER_PERMISSION  "user_permission"
#define DEF_DB_TABLE_USER_AUTHTOKEN   "user_authtoken"
#define DEF_DB_TABLE_USER_BUDDY       "user_buddy"


#define DEF_AUTHTOKEN_TIMEOUT   3600

static struct event* g_ev_validate_authtoken = NULL;

static bool init_databases(void);
static bool init_database_contact(void);
static bool init_database_permission(void);
static bool init_database_authtoken(void);
static bool init_database_userinfo(void);
static bool init_database_buddy(void);

static bool db_create_buddy_info(const json_t* j_data);
static json_t* db_get_buddies_info_by_owneruuid(const char* uuid_user);
static bool db_update_buddy_info(const json_t* j_data);
static bool db_delete_buddy_info(const char* uuid);

static bool db_create_authtoken_info(const json_t* j_data);
static bool db_update_authtoken_info(const json_t* j_data);
static bool db_delete_authtoken_info(const char* key);

static bool db_create_permission_info(const json_t* j_data);
static bool db_delete_permission_info(const char* key);

static bool db_create_contact_info(const json_t* j_data);
static bool db_update_contact_info(const json_t* j_data);

static char* create_authtoken(const char* username, const char* password, const char* type);

static bool db_create_userinfo_info(const json_t* j_data);
static bool db_update_userinfo_info(const json_t* j_data);

static bool create_userinfo(json_t* j_data);
static bool update_userinfo(const char* uuid, json_t* j_data);

static bool create_permission(const char* user_uuid, const char* permission);
static bool create_permission_by_json(json_t* j_data);

static bool create_contact(const json_t* j_data);
static bool update_contact(const char* uuid, json_t* j_data);

static bool create_buddy(const char* uuid, const json_t* j_data);
static json_t* get_buddy(const char* uuid);
static bool update_buddy(const json_t* j_data);

static bool is_user_exist(const char* user_uuid);
static bool is_user_exsit_by_username(const char* username);
static bool is_valid_type_target(const char* type, const char* target);

static void cb_user_validate_authtoken(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);


bool user_init_handler(void)
{
  int ret;
  json_t* j_tmp;
  struct timeval tm_slow;

	slog(LOG_INFO, "Fired init_user_handler.");

	ret = init_databases();
	if(ret == false) {
	  slog(LOG_ERR, "Could not initate database.");
	  return false;
	}

	// test code..

	// create default user admin
	j_tmp = json_pack("{s:s, s:s}",
	    "username", "admin",
	    "password", "admin"
	    );
	create_userinfo(j_tmp);
	json_decref(j_tmp);

	// create default permission admin
	j_tmp = user_get_userinfo_info_by_username_password("admin", "admin");
	if(j_tmp != NULL) {
	  create_permission(json_string_value(json_object_get(j_tmp, "uuid")), DEF_USER_PERM_ADMIN);
	  create_permission(json_string_value(json_object_get(j_tmp, "uuid")), DEF_USER_PERM_USER);
	  json_decref(j_tmp);
	}


	// register event
  tm_slow.tv_sec = 10;
  tm_slow.tv_usec = 0;
	g_ev_validate_authtoken = event_new(g_app->evt_base, -1, EV_TIMEOUT | EV_PERSIST, cb_user_validate_authtoken, NULL);
  event_add(g_ev_validate_authtoken, &tm_slow);

	return true;
}

void user_term_handler(void)
{
	slog(LOG_INFO, "Fired term_user_handler.");

	event_del(g_ev_validate_authtoken);
	event_free(g_ev_validate_authtoken);

	return;
}

bool user_reload_handler(void)
{
	slog(LOG_INFO, "Fired reload_user");

	user_term_handler();
	user_init_handler();

	return true;
}

static bool init_databases(void)
{
  int ret;

  // contact
  ret = init_database_contact();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database contact.");
    return false;
  }

  // permission
  ret = init_database_permission();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database permission.");
    return false;
  }

  // authtoken
  ret = init_database_authtoken();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database authtoken.");
    return false;
  }

  // userinfo
  ret = init_database_userinfo();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database userinfo.");
    return false;
  }

  // buddy
  ret = init_database_buddy();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database buddy.");
    return false;
  }

  return true;
}

/**
 * Initiate contact database.
 * @return
 */
static bool init_database_contact(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_USER_CONTACT " ("

    "   uuid        varchar(255),"
    "   user_uuid   varchar(255),"

    "   type    varchar(255),"    // sip_peer, pjsip_endpoint, ...
    "   target  varchar(255),"    // peer name, endpoint name, ...

    "   name    varchar(255),"
    "   detail  varchar(1023),"

    // timestamp. UTC."
    "   tm_create         datetime(6),"   // create time
    "   tm_update         datetime(6),"   // update time.

    "   primary key(uuid)"
    ");";

  // execute
  ret = resource_exec_file_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_CONTACT);
    return false;
  }

  return true;
}

/**
 * Initiate permission database.
 * @return
 */
static bool init_database_permission(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_USER_PERMISSION " ("

    "   uuid        varchar(255),"
    "   user_uuid   varchar(255),"
    "   permission  varchar(255),"

    "   primary key(user_uuid, permission)"

    ");";

  // execute
  ret = resource_exec_file_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_PERMISSION);
    return false;
  }

  return true;
}

/**
 * Initiate authtoken database.
 * @return
 */
static bool init_database_authtoken(void)
{
  int ret;
  const char* create_table;

  create_table =
    "create table if not exists " DEF_DB_TABLE_USER_AUTHTOKEN " ("

    // identity
    "   uuid        varchar(255),"

    "   user_uuid   varchar(255),"    // user uuid
    "   type        varchar(255),"    // create module name. (me, admin, ...)

    // timestamp. UTC."
    "   tm_create         datetime(6),"   // create time
    "   tm_update         datetime(6),"   // update time.

    "   primary key(uuid)"
    ");";

  // execute
  ret = resource_exec_file_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_AUTHTOKEN);
    return false;
  }

  return true;
}

/**
 * Initiate userinfo database.
 * @return
 */
static bool init_database_userinfo(void)
{
  int ret;
  const char* create_table;

  create_table =
      "create table if not exists " DEF_DB_TABLE_USER_USERINFO " ("

      // identity
      "   uuid        varchar(255),"
      "   username    varchar(255),"
      "   password    varchar(255),"

      // info
      "   name      varchar(255),"

      // timestamp. UTC."
      "   tm_create         datetime(6),"   // create time
      "   tm_update         datetime(6),"   // update time.

      "   primary key(uuid)"
      ");";

  // execute
  ret = resource_exec_file_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_USERINFO);
    return false;
  }

  return true;
}

/**
 * Initiate buddy database.
 * @return
 */
static bool init_database_buddy(void)
{
  int ret;
  const char* create_table;

  create_table =
      "create table if not exists " DEF_DB_TABLE_USER_BUDDY " ("

      // identity
      "   uuid          varchar(255),"
      "   uuid_owner    varchar(255),"  // owner
      "   uuid_user     varchar(255),"  // owner's buddy

      // info
      "   name      varchar(255),"
      "   detail    varchar(255),"

      // timestamp. UTC."
      "   tm_create         datetime(6),"   // create time
      "   tm_update         datetime(6),"   // update time.

      "   primary key(uuid_owner, uuid_user)"
      ");";

  // execute
  ret = resource_exec_file_sql(create_table);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate database. database[%s]", DEF_DB_TABLE_USER_BUDDY);
    return false;
  }

  return true;
}

/**
 *  @brief  Check the user_authtoken and validate
 */
static void cb_user_validate_authtoken(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_auths;
  json_t* j_auth;
  const char* last_update;
  const char* uuid;
  int idx;
  char* timestamp;
  time_t time_over;
  time_t time_update;

  // get all authtoken info
  j_auths = user_get_authtokens_all();
  if(j_auths == NULL) {
    slog(LOG_ERR, "Could not get authtoken info.");
    return;
  }

  // get curtime
  timestamp = utils_get_utc_timestamp();
  time_over = utils_get_unixtime_from_utc_timestamp(timestamp);
  sfree(timestamp);
  time_over -= DEF_AUTHTOKEN_TIMEOUT;

  json_array_foreach(j_auths, idx, j_auth) {

    uuid = json_string_value(json_object_get(j_auth, "uuid"));

    // get last updated time
    last_update = json_string_value(json_object_get(j_auth, "tm_update"));
    if(last_update == NULL) {
      slog(LOG_ERR, "Could not get tm_update info. Remove authtoken. uuid[%s]", uuid);
      user_delete_authtoken_info(uuid);
      continue;
    }

    // convert
    time_update = utils_get_unixtime_from_utc_timestamp(last_update);
    if(time_update == 0) {
      slog(LOG_ERR, "Could not convert tm_update info. Remove authtoken. uuid[%s]", uuid);
      user_delete_authtoken_info(uuid);
      continue;
    }

    // check timeout
    if(time_update < time_over) {
      slog(LOG_NOTICE, "The authtoken is timed out. Remove authtoken. uuid[%s]", uuid);
      user_delete_authtoken_info(uuid);
      continue;
    }
  }

  json_decref(j_auths);
  return;
}

/**
 * htp request handler.
 * request: POST ^/user/login$
 * @param req
 * @param data
 */
void user_htp_post_user_login(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  char* username;
  char* password;
  char* authtoken;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_entries.");

  // get username/pass
  ret = http_get_htp_id_pass(req, &username, &password);
  if(ret == false) {
    sfree(username);
    sfree(password);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create authtoken
  authtoken = create_authtoken(username, password, "admin");
  sfree(username);
  sfree(password);
  if(authtoken == NULL) {
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  j_tmp = json_pack("{s:s}",
      "authtoken",  authtoken
      );
  sfree(authtoken);

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
 * request: DELETE ^/user/login$
 * @param req
 * @param data
 */
void user_htp_delete_user_login(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* authtoken;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_user_login.");

  // get authtoken
  authtoken = evhtp_kv_find(req->uri->query, "authtoken");
  if(authtoken == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete authtoken
  ret = user_delete_authtoken_info(authtoken);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_res = http_create_default_result(EVHTP_RES_OK);
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/user/contacts request handler.
 * @param req
 * @param data
 */
void user_htp_get_user_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_contacts.");

  // get info
  j_tmp = user_get_contacts_all();
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
 * htp request handler.
 * request: POST ^/user/contacts
 * @param req
 * @param data
 */
void user_htp_post_user_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_user_contacts.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create contact
  ret = create_contact(j_data);
  json_decref(j_data);
  if(ret == false) {
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
 * GET ^/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void user_htp_get_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_contacts_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = user_get_contact_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find contact info.");
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
 * PUT ^/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void user_htp_put_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_user_contacts_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_NOTICE, "Could not get data info.");
    sfree(detail);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update
  ret = update_contact(detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
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
 * DELETE ^/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void user_htp_delete_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_user_contacts_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = user_delete_contact_info(detail);
  sfree(detail);
  if(ret == false) {
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
 * request: POST ^/user/users
 * @param req
 * @param data
 */
void user_htp_post_user_users(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_user_users.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create user
  ret = create_userinfo(j_data);
  json_decref(j_data);
  if(ret == false) {
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
 * request: GET ^/user/users
 * @param req
 * @param data
 */
void user_htp_get_user_users(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_users.");

  // get info
  j_tmp = user_get_userinfos_all();
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
 * GET ^/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void user_htp_get_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_users_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = user_get_userinfo_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find userinfo info.");
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
 * PUT ^/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void user_htp_put_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_user_users_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_NOTICE, "Could not get data info.");
    sfree(detail);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update
  ret = update_userinfo(detail, j_data);
  sfree(detail);
  json_decref(j_data);
  if(ret == false) {
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
 * DELETE ^/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void user_htp_delete_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_user_users_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = user_delete_userinfo_info(detail);
  sfree(detail);
  if(ret == false) {
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
 * GET ^/user/permissions request handler.
 * @param req
 * @param data
 */
void user_htp_get_user_permissions(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_user_permissions.");

  // get info
  j_tmp = user_get_permissions_all();
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
 * htp request handler.
 * request: POST ^/user/permissions
 * @param req
 * @param data
 */
void user_htp_post_user_permissions(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_user_permissions.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create data
  ret = create_permission_by_json(j_data);
  json_decref(j_data);
  if(ret == false) {
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
 * DELETE ^/user/permissions/(.*) request handler.
 * @param req
 * @param data
 */
void user_htp_delete_user_permissions_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_user_permissions_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = user_delete_permission_info(detail);
  sfree(detail);
  if(ret == false) {
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

static char* create_authtoken(const char* username, const char* password, const char* type)
{
  json_t* j_user;
  json_t* j_auth;
  char* token;
  char* timestamp;

  if((username == NULL) || (password == NULL) || (type == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired create_authtoken. username[%s], password[%s]", username, "*");

  // get user info
  j_user = user_get_userinfo_info_by_username_password(username, password);
  if(j_user == NULL) {
    slog(LOG_INFO, "Could not find correct userinfo of given data. username[%s], password[%s]", username, "*");
    return NULL;
  }

  // create
  token = utils_gen_uuid();
  timestamp = utils_get_utc_timestamp();
  j_auth = json_pack("{s:s, s:s, s:s, s:s, s:s}",
      "uuid",       token,
      "user_uuid",  json_string_value(json_object_get(j_user, "uuid")),
      "type",       type,

      "tm_create",  timestamp,
      "tm_update",  timestamp
      );
  sfree(timestamp);
  json_decref(j_user);

  // create auth info
  db_create_authtoken_info(j_auth);
  json_decref(j_auth);

  return token;
}

static bool create_permission_by_json(json_t* j_data)
{
  const char* user_uuid;
  const char* permission;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  user_uuid = json_string_value(json_object_get(j_data, "user_uuid"));
  permission = json_string_value(json_object_get(j_data, "permission"));

  ret = create_permission(user_uuid, permission);
  if(ret == false) {
    slog(LOG_ERR, "Could not create permission by json.");
    return false;
  }

  return true;
}

static bool create_permission(const char* user_uuid, const char* permission)
{
  json_t* j_tmp;
  char* uuid;
  int ret;

  if((user_uuid == NULL) || (permission == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  uuid = utils_gen_uuid();
  j_tmp = json_pack("{s:s, s:s, s:s}",
      "uuid",       uuid,
      "user_uuid",  user_uuid,
      "permission", permission
      );
  sfree(uuid);

  ret = user_create_permission_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool create_contact(const json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  char* uuid;
  const char* user_uuid;
  const char* type;
  const char* target;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_contact.");

  // validate user_uuid
  user_uuid = json_string_value(json_object_get(j_data, "user_uuid"));
  ret = is_user_exist(user_uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "User is not exist. user_uuid[%s]", user_uuid);
    return false;
  }

  // validate type target
  type = json_string_value(json_object_get(j_data, "type"));
  target = json_string_value(json_object_get(j_data, "target"));
  ret = is_valid_type_target(type, target);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not pass the type target validation. type[%s], target[%s]", type?:"", target?:"");
    return false;
  }

  // create contact info
  timestamp = utils_get_utc_timestamp();
  uuid = utils_gen_uuid();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, s:s, s:s, "
      "s:s "
      "}",

      "uuid",         uuid,
      "user_uuid",    user_uuid,

      "type",         type,
      "target",       target,
      "name",         json_string_value(json_object_get(j_data, "name"))? : "",
      "detail",       json_string_value(json_object_get(j_data, "detail"))? : "",

      "tm_create",    timestamp
      );
  sfree(timestamp);
  sfree(uuid);

  // create resource
  ret = db_create_contact_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create user contact info.");
    return false;
  }

  return true;
}

static bool update_contact(const char* uuid, json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  int ret;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_contact. uuid[%s]", uuid);

  timestamp = utils_get_utc_timestamp();
  j_tmp = json_pack("{"
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s, s:s, "
      "s:s "
      "}",

      "uuid",       uuid,
      "user_uuid",  json_string_value(json_object_get(j_data, "user_uuid"))? : "",

      "type",     json_string_value(json_object_get(j_data, "type"))? : "",
      "target",   json_string_value(json_object_get(j_data, "target"))? : "",

      "name",     json_string_value(json_object_get(j_data, "name"))? : "",
      "detail",   json_string_value(json_object_get(j_data, "detail"))? : "",

      "tm_update", timestamp
      );
  sfree(timestamp);

  ret = db_update_contact_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update contact info.");
    return false;
  }

  return true;
}

/**
 * Create user info.
 * @param j_data
 * @return
 */
static bool create_userinfo(json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  char* uuid;
  const char* username;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user.");

  // validate info.
  username = json_string_value(json_object_get(j_data, "username"));
  if(username == NULL) {
    slog(LOG_ERR, "Could not get username info.");
    return false;
  }

  // check exist
  ret = is_user_exsit_by_username(username);
  if(ret == true) {
    slog(LOG_ERR, "The given username is already exist. username[%s]", username);
    return false;
  }

  // create contact info
  timestamp = utils_get_utc_timestamp();
  uuid = utils_gen_uuid();
  j_tmp = json_pack("{"
      "s:s, "
      "s:s, s:s, "
      "s:s, "
      "s:s "
      "}",

      "uuid",         uuid,

      "username",     username,
      "password",     json_string_value(json_object_get(j_data, "password"))? : "",

      "name",         json_string_value(json_object_get(j_data, "name"))? : "",

      "tm_create",    timestamp
      );
  sfree(timestamp);
  sfree(uuid);

  // create resource
  ret = db_create_userinfo_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create user contact info.");
    return false;
  }

  return true;
}

/**
 * Update userinfo.
 * @param uuid
 * @param j_data
 * @return
 */
static bool update_userinfo(const char* uuid, json_t* j_data)
{
  json_t* j_tmp;
  char* timestamp;
  int ret;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_contact. uuid[%s]", uuid);

  timestamp = utils_get_utc_timestamp();
  j_tmp = json_deep_copy(j_data);

  json_object_del(j_tmp, "username");
  json_object_del(j_tmp, "tm_update");
  json_object_del(j_tmp, "tm_create");

  timestamp = utils_get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_update", json_string(timestamp));
  sfree(timestamp);

  ret = db_update_userinfo_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not update contact info.");
    return false;
  }

  return true;
}

/**
 * Return true if given user_uuid is exist in user_user
 * @param user_uuid
 * @return
 */
bool user_is_user_exist(const char* user_uuid)
{
  int ret;

  if(user_uuid == NULL) {
    return false;
  }

  ret = is_user_exist(user_uuid);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool is_user_exist(const char* user_uuid)
{
  json_t* j_tmp;

  if(user_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = user_get_userinfo_info(user_uuid);
  if(j_tmp == NULL) {
    return false;
  }

  json_decref(j_tmp);
  return true;
}

static bool is_user_exsit_by_username(const char* username)
{
  json_t* j_tmp;

  if(username == NULL) {
    slog(LOG_WARNING, "Wrong input paramter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired is_user_exsit_by_username. username[%s]", username);

  j_tmp = user_get_userinfo_info_by_username(username);
  if(j_tmp == NULL) {
    return false;
  }

  json_decref(j_tmp);
  return true;
}

static bool is_valid_type_target(const char* type, const char* target)
{
  json_t* j_tmp;

  if((type == NULL) || (target == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // validate
  if(strcmp(type, DEF_USER_CONTACT_TYPE_PEER) == 0) {
    // peer type

    j_tmp = sip_get_peer_info(target);
    if(j_tmp == NULL) {
      return false;
    }

    json_decref(j_tmp);
    return true;
  }
  else if(strcmp(type, DEF_USER_CONTACT_TYPE_ENDPOINT) == 0) {
    // pjsip type

    j_tmp = pjsip_get_endpoint_info(target);
    if(j_tmp == NULL) {
      return false;
    }

    json_decref(j_tmp);
    return true;
  }
  else {
    // wrong type

    return false;
  }

  // should not reach to here
  slog(LOG_ERR, "Should not reach to here. Something was wrong.");
  return false;
}

/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* user_get_userinfo_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_info. key[%s]", key);

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_USER_USERINFO, "uuid", key);

  return j_res;
}

/**
 * Get all user_userinfo detail info.
 * @return
 */
json_t* user_get_userinfos_all(void)
{
  json_t* j_res;

  slog(LOG_DEBUG, "Fired get_user_userinfos_all.");

  j_res = resource_get_file_items(DEF_DB_TABLE_USER_USERINFO, "*");

  return j_res;
}


/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* user_get_userinfo_info_by_username(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_info_by_username. key[%s]", key);

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_USER_USERINFO, "username", key);

  return j_res;
}

/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* user_get_userinfo_info_by_username_password(const char* username, const char* pass)
{
  json_t* j_res;
  json_t* j_obj;

  if((username == NULL) || (pass == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_info_by_username_pass. username[%s], pass[%s]", username, "*");

  j_obj = json_pack("{s:s, s:s}",
      "username",   username,
      "password",   pass
      );

  j_res = resource_get_file_detail_item_by_obj(DEF_DB_TABLE_USER_USERINFO, j_obj);
  json_decref(j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* user_get_userinfo_by_authtoken(const char* authtoken)
{
  json_t* j_auth;
  json_t* j_user;
  const char* user_uuid;

  if(authtoken == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_userinfo_by_authtoken. authtoken[%s]", authtoken);

  j_auth = user_get_authtoken_info(authtoken);
  if(j_auth == NULL) {
    slog(LOG_ERR, "Could not get authtoken info.");
    return NULL;
  }

  user_uuid = json_string_value(json_object_get(j_auth, "user_uuid"));
  if(user_uuid == NULL) {
    slog(LOG_ERR, "Could not get user_uuid.");
    json_decref(j_auth);
    return NULL;
  }

  j_user = user_get_userinfo_info(user_uuid);
  json_decref(j_auth);
  if(j_user == NULL) {
    slog(LOG_ERR, "Could not get user info.");
    return NULL;
  }

  return j_user;
}

static bool db_create_userinfo_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_userinfo_info.");

  // insert userinfo info
  ret = resource_insert_file_item(DEF_DB_TABLE_USER_USERINFO, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_userinfo.");
    return false;
  }

  return true;
}

static bool db_update_userinfo_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_update_userinfo_info.");

  // insert userinfo info
  ret = resource_update_file_item(DEF_DB_TABLE_USER_USERINFO, "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_userinfo.");
    return false;
  }

  return true;
}

bool user_update_userinfo_info(const char* uuid_user, const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if((uuid_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_userinfo_info.");

  // create update data
  j_tmp = json_object();

  tmp_const = json_string_value(json_object_get(j_data, "name"));
  if(tmp_const != NULL) {
    json_object_set_new(j_tmp, "name", json_string(tmp_const));
  }

  tmp_const = json_string_value(json_object_get(j_data, "password"));
  if(tmp_const != NULL) {
    json_object_set_new(j_tmp, "password", json_string(tmp_const));
  }

  // update info
  ret = update_userinfo(uuid_user, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_WARNING, "Could not update userinfo.");
    return false;
  }

  return true;
}

/**
 * Delete user_userinfo info.
 * @return
 */
bool user_delete_userinfo_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_user_userinfo_info. key[%s]", key);

  ret = resource_delete_file_items_string(DEF_DB_TABLE_USER_USERINFO, "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete dp_dialplan info. key[%s]", key);
    return false;
  }

  return true;
}

/**
 * Get corresponding user_authtoken all detail info.
 * @return
 */
json_t* user_get_authtokens_all(void)
{
  json_t* j_res;

  j_res = resource_get_file_items(DEF_DB_TABLE_USER_AUTHTOKEN, "*");
  return j_res;
}

/**
 * Get all authtokens of given user uuid.
 * @param uuid_user
 * @return
 */
json_t* user_get_authtokens_user_type(const char* uuid_user, const char* type)
{
  json_t* j_res;
  json_t* j_tmp;

  if((uuid_user == NULL) || (type == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired user_get_authtokens_user_type. uuid_user[%s], type[%s]", uuid_user, type);

  j_tmp = json_pack("{s:s, s:s}",
      "user_uuid",    uuid_user,
      "type",         type
      );

  j_res = resource_get_file_detail_items_by_obj(DEF_DB_TABLE_USER_AUTHTOKEN, j_tmp);
  json_decref(j_tmp);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Get corresponding user_authtoken detail info.
 * @return
 */
json_t* user_get_authtoken_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_authtoken_info. key[%s]", key);

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_USER_AUTHTOKEN, "uuid", key);

  return j_res;
}

static bool db_create_authtoken_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_create_authtoken_info.");

  // insert authtoken info
  ret = resource_insert_file_item(DEF_DB_TABLE_USER_AUTHTOKEN, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_authtoken.");
    return false;
  }

  return true;
}

static bool db_update_authtoken_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_update_authtoken_info.");

  // update info
  ret = resource_update_file_item(DEF_DB_TABLE_USER_AUTHTOKEN, "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update user_authtoken info.");
    return false;
  }

  return true;
}

/**
 * Delete user_authtoken info.
 * @param key
 * @return
 */
static bool db_delete_authtoken_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_delete_authtoken_info. key[%s]", key);

  ret = resource_delete_file_items_string(DEF_DB_TABLE_USER_AUTHTOKEN, "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete user_authtoken info. key[%s]", key);
    return false;
  }

  return true;
}

static bool db_create_permission_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired db_create_permission_info.");

  // insert permission info
  ret = resource_insert_file_item(DEF_DB_TABLE_USER_PERMISSION, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_permission.");
    return false;
  }

  return true;
}

static bool db_delete_permission_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  // delete
  ret = resource_delete_file_items_string(DEF_DB_TABLE_USER_PERMISSION, "uuid", key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete permission info. uuid[%s]", key);
    return false;
  }

  return true;
}

static bool db_create_contact_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_contact_info.");

  // insert info
  ret = resource_insert_file_item(DEF_DB_TABLE_USER_CONTACT, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_contact.");
    return false;
  }

  return true;
}

static json_t* db_get_buddies_info_by_owneruuid(const char* uuid_user)
{
  json_t* j_res;

  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = resource_get_file_detail_items_key_string(DEF_DB_TABLE_USER_BUDDY, "uuid_owner", uuid_user);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool db_create_buddy_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  ret = resource_insert_file_item(DEF_DB_TABLE_USER_BUDDY, j_data);
  if(ret == false) {
    slog(LOG_WARNING, "Could not insert data into table.");
    return false;
  }

  return true;
}

static bool db_delete_buddy_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_delete_file_items_string(DEF_DB_TABLE_USER_BUDDY, "uuid", uuid);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool db_update_buddy_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = resource_update_file_item(DEF_DB_TABLE_USER_BUDDY, "uuid", j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

bool user_update_authtoken_tm_update(const char* uuid)
{
  json_t* j_token;
  char* timestamp;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_token = user_get_authtoken_info(uuid);
  if(j_token == NULL) {
    slog(LOG_NOTICE, "Could not get authtoken info. uuid[%s]", uuid);
    return false;
  }

  timestamp = utils_get_utc_timestamp();
  json_object_set_new(j_token, "tm_update", json_string(timestamp));
  sfree(timestamp);

  ret = db_update_authtoken_info(j_token);
  json_decref(j_token);
  if(ret == false) {
    slog(LOG_ERR, "Could not update authtoken info. uuid[%s]", uuid);
    return false;
  }

  return true;
}

/**
 * Delete user_authtoken info.
 * @param key
 * @return
 */
bool user_delete_authtoken_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_user_authtoken_info. key[%s]", key);

  ret = db_delete_authtoken_info(key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete user_authtoken info. key[%s]", key);
    return false;
  }

  return true;
}

bool user_create_permission_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_permission_info.");

  ret = db_create_permission_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_authtoken.");
    return false;
  }

  return true;
}

json_t* user_get_permissions_all(void)
{
  json_t* j_res;

  j_res = resource_get_file_items(DEF_DB_TABLE_USER_PERMISSION, "*");
  return j_res;
}

/**
 * Delete user_permission info.
 * @param key
 * @return
 */
bool user_delete_permission_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_user_permission_info. uuid[%s]", key);

  ret = db_delete_permission_info(key);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete permission info. uuid[%s]", key);
    return false;
  }

  return true;
}

/**
 * Get corresponding user_userinfo detail info.
 * @return
 */
json_t* user_get_permission_info_by_useruuid_perm(const char* useruuid, const char* perm)
{
  json_t* j_res;
  json_t* j_obj;

  if((useruuid == NULL) || (perm == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_permission_info_by_useruuid_perm. useruuid[%s], perm[%s]", useruuid, perm);

  j_obj = json_pack("{s:s, s:s}",
      "user_uuid",    useruuid,
      "permission",   perm
      );

  j_res = resource_get_file_detail_item_by_obj(DEF_DB_TABLE_USER_PERMISSION, j_obj);
  json_decref(j_obj);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

json_t* user_get_contacts_all(void)
{
  json_t* j_res;

  j_res = resource_get_file_items(DEF_DB_TABLE_USER_CONTACT, "*");
  return j_res;
}

/**
 * Returns conntact info array of given user_uuid.
 * @param user_uuid
 * @return
 */
json_t* user_get_contacts_by_user_uuid(const char* user_uuid)
{
  json_t* j_res;
  json_t* j_obj;

  if(user_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_obj = json_pack("{s:s}",
      "user_uuid",  user_uuid
      );

  j_res = resource_get_file_detail_items_by_obj(DEF_DB_TABLE_USER_CONTACT, j_obj);
  json_decref(j_obj);

  return j_res;
}

/**
 * Get corresponding user_contact detail info.
 * @return
 */
json_t* user_get_contact_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_user_contact_info. key[%s]", key);

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_USER_CONTACT, "uuid", key);

  return j_res;
}

/**
 * Interface for create contact info.
 * @param j_data
 * @return
 */
bool user_create_contact_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_user_contact_info.");

  // create info
  ret = create_contact(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not insert user_contact.");
    return false;
  }

  return true;
}

/**
 * Interface for update contact info.
 * @param j_data
 * @return
 */
static bool db_update_contact_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_user_contact_info.");

  // update info
  ret = resource_update_file_item(DEF_DB_TABLE_USER_CONTACT, "uuid", j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update user_contact info.");
    return false;
  }

  return true;
}

/**
 * Delete user_contact info.
 * @param key
 * @return
 */
bool user_delete_contact_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_user_contact_info. key[%s]", key);

  ret = resource_delete_file_items_string(DEF_DB_TABLE_USER_CONTACT, "uuid", key);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete user_contact info. key[%s]", key);
    return false;
  }

  return true;
}

/**
 * Return array of given user's buddies
 * @param uuid_user
 * @return
 */
json_t* user_get_buddies_info_by_owneruuid(const char* uuid_user)
{
  json_t* j_res;

  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Wrong inpua parameter.");
    return NULL;
  }

  j_res = db_get_buddies_info_by_owneruuid(uuid_user);
  if(j_res == NULL) {
    slog(LOG_WARNING, "Could not get buddies info.");
    return NULL;
  }

  return j_res;
}

/**
 * Return the buddy info of given data.
 * @param uuid
 * @return
 */
json_t* user_get_buddy_info(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_USER_BUDDY, "uuid", uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Interface for create buddy info.
 * @param uuid
 * @param j_data
 * @return
 */
bool user_create_buddy_info(const char* uuid, const json_t* j_data)
{
  int ret;
  const char* uuid_owner;
  const char* uuid_user;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get owner
  uuid_owner = json_string_value(json_object_get(j_data, "uuid_owner"));
  if(uuid_owner == NULL) {
    slog(LOG_WARNING, "Could not get owner uuid.");
    return false;
  }

  ret = is_user_exist(uuid_owner);
  if(ret == false) {
    slog(LOG_WARNING, "The owner is not exist.");
    return false;
  }

  // get user
  uuid_user = json_string_value(json_object_get(j_data, "uuid_user"));
  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Could not get user uuid.");
    return false;
  }

  ret = is_user_exist(uuid_user);
  if(ret == false) {
    slog(LOG_WARNING, "The user is not exsit.");
    return false;
  }

  // create
  ret = create_buddy(uuid, j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Delete given info from buddy.
 * @param uuid
 * @return
 */
bool user_delete_buddy_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = db_delete_buddy_info(uuid);
  if(ret == false) {
    return false;
  }

  return true;
}

bool user_update_buddy_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = update_buddy(j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Create buddy.
 * @param uuid
 * @param j_data
 * @return
 */
static bool create_buddy(const char* uuid, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  char* timestamp;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_deep_copy(j_data);

  timestamp = utils_get_utc_timestamp();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));
  json_object_set_new(j_tmp, "tm_create", json_string(timestamp));
  json_object_del(j_tmp, "tm_update");
  sfree(timestamp);

  // create
  ret = db_create_buddy_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool update_buddy(const json_t* j_data)
{
  int ret;
  char* timestamp;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  timestamp = utils_get_utc_timestamp();

  j_tmp = json_deep_copy(j_data);
  json_object_set_new(j_tmp, "tm_update", json_string(timestamp));
  json_object_del(j_tmp, "tm_create");
  sfree(timestamp);

  // update
  ret = db_update_buddy_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_buddy(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = resource_get_file_detail_item_key_string(DEF_DB_TABLE_USER_BUDDY, "uuid", uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

/**
 * Return true if given user owned given buddy.
 * Otherwise, return false.
 * @param uuid_owner
 * @param uuid_buddy
 * @return
 */
bool user_is_user_owned_buddy(const char* uuid_owner, const char* uuid_buddy)
{
  int ret;
  json_t* j_tmp;
  const char* tmp_const;

  if((uuid_owner == NULL) || (uuid_buddy == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = get_buddy(uuid_buddy);
  if(j_tmp == NULL) {
    return false;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid_owner"));
  if(tmp_const == NULL) {
    json_decref(j_tmp);
    return false;
  }

  ret = strcmp(tmp_const, uuid_owner);
  json_decref(j_tmp);
  if(ret != 0) {
    return false;
  }

  return true;
}

char* user_create_authtoken(const char* username, const char* password, const char* type)
{
  char* res;

  if((username == NULL) || (password == NULL) || (type == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  res = create_authtoken(username, password, type);
  if(res == NULL) {
    return NULL;
  }

  return res;
}

