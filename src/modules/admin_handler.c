/*
 * admin_handler.c
 *
 *  Created on: Apr 19, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <evhtp.h>
#include <jansson.h>

#include "slog.h"
#include "utils.h"

#include "http_handler.h"
#include "user_handler.h"
#include "queue_handler.h"
#include "park_handler.h"
#include "core_handler.h"
#include "call_handler.h"
#include "publication_handler.h"

#include "admin_handler.h"

#define DEF_ADMIN_AUTHTOKEN_TYPE       "admin"

#define DEF_PUBLISH_TOPIC_PREFIX_ADMIN     "/admin"

#define DEF_PUB_EVENT_PREFIX_ADMIN_CORE_CHANNEL   "admin.core.channel"
#define DEF_PUB_EVENT_PREFIX_ADMIN_CORE_MODULE    "admin.core.module"
#define DEF_PUB_EVENT_PREFIX_ADMIN_CORE_SYSTEM    "admin.core.system"

#define DEF_PUB_EVENT_PREFIX_ADMIN_PARK_PARKEDCALL   "admin.park.parkedcall"


static bool init_callbacks(void);


////// core module
// channels
static json_t* get_core_channels_all(void);
static json_t* get_core_channel_info(const char* key);
static bool delete_core_channel_info(const char* key);

// modules
static json_t* get_core_modules_all(void);
static json_t* get_core_module_info(const char* key);

// systems
static json_t* get_core_systems_all(void);
static json_t* get_core_system_info(const char* key);



////// queue module
// queues
static json_t* get_queue_queues_all(void);
static json_t* get_queue_queue_info(const char* key);

// members
static json_t* get_queue_members_all(void);
static json_t* get_queue_member_info(const char* key);
static bool create_queue_member_info(const json_t* j_data);
static bool update_queue_member_info(const char* key, const json_t* j_data);
static bool delete_queue_member_info(const char* key);

// entries
static json_t* get_queue_entries_all(void);
static json_t* get_queue_entry_info(const char* key);
static bool delete_queue_entry_info(const char* key);


////// park module
static json_t* get_park_parkedcalls_all(void);
static json_t* get_park_parkedcall_info(const char* key);
static bool delete_park_parkedcall_info(const char* key);

static json_t* get_park_parkinglots_all(void);
static json_t* get_park_parkinglot_info(const char* key);

static json_t* get_park_cfg_parkinglots_all(void);
static json_t* get_park_cfg_parkinglot_info(const char* key);
static bool create_park_cfg_parkinglot_info(const json_t* j_data);
static bool update_park_cfg_parkinglot_info(const char* name, const json_t* j_data);
static bool delete_park_cfg_parkinglot_info(const char* name);

static json_t* get_park_configurations_all(void);
static json_t* get_park_configuration_info(const char* name);
static bool update_park_configuration_info(const char* name, const json_t* j_data);
static bool delete_park_configuration_info(const char* name);


////// user module
// user
static json_t* get_user_users_all(void);
static json_t* get_user_userinfo(const char* uuid);
static bool create_user_userinfo(const json_t* j_data);
static bool update_user_userinfo(const char* uuid, const json_t* j_data);
static bool delete_user_userinfo(const char* uuid);

// contact
static json_t* get_user_contacts_all(void);
static json_t* get_user_contact_info(const char* uuid);
static bool create_user_contact_info(const json_t* j_data);
static bool update_user_contact_info(const char* uuid, const json_t* j_data);
static bool delete_user_contact_info(const char* uuid);

// permission
static json_t* get_user_permissions_all(void);
static json_t* get_user_permission_info(const char* uuid);
static bool create_user_permission_info(const json_t* j_data);
static bool delete_user_permission_info(const char* uuid);


// info
static bool update_admin_info(const json_t* j_user, const json_t* j_data);
static json_t* get_admin_info(const json_t* j_user);


///// callback
static bool cb_resource_handler_core_db_channel(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);
static bool cb_resource_handler_core_db_module(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);
static bool cb_resource_handler_park_db_parkedcall(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);

///// etc
static bool load_module(const char* name);
static bool reload_module(const char* name);
static bool unload_module(const char* name);


bool admin_init_handler(void)
{
  int ret;

  ret = init_callbacks();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate callbacks.");
    return false;
  }

  return true;
}

bool admin_term_handler(void)
{
  return true;
}

bool admin_reload_handler(void)
{
  int ret;

  ret = admin_term_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not terminate admin_handler.");
    return false;
  }

  ret = admin_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate admin_handler.");
    return false;
  }

  return true;
}

static bool init_callbacks(void)
{
  core_register_callback_db_channel(&cb_resource_handler_core_db_channel);
  core_register_callback_db_module(&cb_resource_handler_core_db_module);

  park_register_callback_db_parkedcall(&cb_resource_handler_park_db_parkedcall);


  return true;
}

/**
 * Returns all subscribable topics of admin module.
 * @param j_user
 * @return
 */
json_t* admin_get_subscribable_topics_all(const json_t* j_user)
{
  char* topic;
  json_t* j_res;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = json_array();

  // set topics
  asprintf(&topic, "%s", DEF_PUBLISH_TOPIC_PREFIX_ADMIN);
  json_array_append_new(j_res, json_string(topic));
  sfree(topic);

  return j_res;
}

/**
 * GET ^/admin/info request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_info(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_info.");

  // get userinfo
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_admin_info(j_user);
  json_decref(j_user);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get manager info.");
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
 * PUT ^/admin/info request handler.
 * @param req
 * @param data
 */
void admin_htp_put_admin_info(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  json_t* j_data;
  json_t* j_user;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_put_admin_info.");

  // get userinfo
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    json_decref(j_user);
    return;
  }

  // update info
  ret = update_admin_info(j_user, j_data);
  json_decref(j_data);
  json_decref(j_user);
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
 * POST ^/admin/login request handler.
 * @param req
 * @param data
 */
void admin_htp_post_admin_login(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* username;
  char* password;
  char* authtoken;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_post_admin_login.");

  // get username/pass
  ret = http_get_htp_id_pass(req, &username, &password);
  if(ret == false) {
    sfree(username);
    sfree(password);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create authtoken
  authtoken = user_create_authtoken(username, password, DEF_ADMIN_AUTHTOKEN_TYPE);
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
 * request: DELETE ^/admin/login$
 * @param req
 * @param data
 */
void admin_htp_delete_admin_login(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* authtoken;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_login.");

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
 * GET ^/admin/user/users request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_user_users(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_user_users.");

  // get info
  j_tmp = get_user_users_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get users info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * request: POST ^/admin/user/users$
 * @param req
 * @param data
 */
void admin_htp_post_admin_user_users(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_post_admin_user_users.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create user
  ret = create_user_userinfo(j_data);
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
 * GET ^/admin/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_user_users_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_user_userinfo(detail);
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
 * PUT ^/admin/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_put_admin_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_put_admin_user_users_detail.");

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
  ret = update_user_userinfo(detail, j_data);
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
 * DELETE ^/admin/user/users/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_user_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_user_users_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_user_userinfo(detail);
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
 * GET ^/admin/user/contacts request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_user_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_user_contacts.");

  // get info
  j_tmp = get_user_contacts_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get contacts info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * request: POST ^/admin/user/contacts$
 * @param req
 * @param data
 */
void admin_htp_post_admin_user_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_post_admin_user_contacts.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create info
  ret = create_user_contact_info(j_data);
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
 * GET ^/admin/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_user_contacts_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_user_contact_info(detail);
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
 * PUT ^/admin/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_put_admin_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_put_admin_user_contacts_detail.");

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
  ret = update_user_contact_info(detail, j_data);
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
 * DELETE ^/admin/user/contacts/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_user_contacts_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_user_contacts_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_user_contact_info(detail);
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
 * GET ^/admin/user/permissions$ request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_user_permissions(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_user_permissions.");

  // get info
  j_tmp = get_user_permissions_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * request: POST ^/admin/user/permissions$
 * @param req
 * @param data
 */
void admin_htp_post_admin_user_permissions(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_post_admin_user_permissions.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create info
  ret = create_user_permission_info(j_data);
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
 * GET ^/admin/user/permissions/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_user_permissions_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_user_permissions_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_user_permission_info(detail);
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
 * DELETE ^/admin/user/permissions/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_user_permissions_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_user_permissions_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_user_permission_info(detail);
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
 * GET ^/admin/queue/queues request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_queue_queues(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_queue_queues.");

  // get info
  j_tmp = get_queue_queues_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/queue/queues/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_queue_queues_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_queue_queues_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_queue_queue_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * GET ^/admin/queue/members request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_queue_members(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_queue_members.");

  // get info
  j_tmp = get_queue_members_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * request: POST ^/admin/queue/members$
 * @param req
 * @param data
 */
void admin_htp_post_admin_queue_members(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_post_admin_queue_members.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create info
  ret = create_queue_member_info(j_data);
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
 * GET ^/admin/queue/members/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_queue_members_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_queue_members_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_queue_member_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * PUT ^/admin/queue/members/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_put_admin_queue_members_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_put_admin_queue_members_detail.");

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
  ret = update_queue_member_info(detail, j_data);
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
 * DELETE ^/admin/queue/members/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_queue_member_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_queue_member_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_queue_member_info(detail);
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
 * GET ^/admin/queue/entries request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_queue_entries(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_queue_entries.");

  // get info
  j_tmp = get_queue_entries_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/queue/entries/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_queue_entries_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_queue_entries_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_queue_entry_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * DELETE ^/admin/queue/entries/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_queue_entries_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_queue_entries_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_queue_entry_info(detail);
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
 * GET ^/admin/park/parkinglots request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_parkinglots(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_parkinglots.");

  // get info
  j_tmp = get_park_parkinglots_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/park/parkinglots/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_parkinglots_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_parkinglots_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_park_parkinglot_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * GET ^/admin/park/cfg_parkinglots request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_cfg_parkinglots(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_cfg_parkinglots.");

  // get info
  j_tmp = get_park_cfg_parkinglots_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * request: POST ^/admin/park/cfg_parkinglots$
 * @param req
 * @param data
 */
void admin_htp_post_admin_park_cfg_parkinglots(evhtp_request_t *req, void *data)
{
  json_t* j_data;
  json_t* j_res;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_post_admin_park_cfg_parkinglots.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create info
  ret = create_park_cfg_parkinglot_info(j_data);
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
 * GET ^/admin/park/cfg_parkinglots/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_cfg_parkinglots_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_cfg_parkinglots_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_park_cfg_parkinglot_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * PUT ^/admin/park/cfg_parkinglots/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_put_admin_park_cfg_parkinglots_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_put_admin_park_cfg_parkinglots_detail.");

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
  ret = update_park_cfg_parkinglot_info(detail, j_data);
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
 * DELETE ^/admin/park/cfg_parkinglots/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_park_cfg_parkinglots_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_park_cfg_parkinglots_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_park_cfg_parkinglot_info(detail);
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
 * GET ^/admin/park/parkedcalls request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_parkedcalls(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_parkedcalls.");

  // get info
  j_tmp = get_park_parkedcalls_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/park/parkedcalls/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_parkedcalls_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_parkedcalls_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_park_parkedcall_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * DELETE ^/admin/park/parkinglots/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_park_parkinglots_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_park_parkinglots_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_park_parkedcall_info(detail);
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
 * GET ^/admin/park/configurations request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_configurations(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_configurations.");

  // get info
  j_tmp = get_park_configurations_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/park/configurations/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_park_configurations_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_park_configurations_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_park_configuration_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * PUT ^/admin/park/configurations/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_put_admin_park_configurations_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_put_admin_park_configurations_detail.");

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
  ret = update_park_configuration_info(detail, j_data);
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
 * DELETE ^/admin/park/configurations/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_park_configurations_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_park_configurations_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_park_configuration_info(detail);
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
 * GET ^/admin/core/channels request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_core_channels(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_core_channels.");

  // get info
  j_tmp = get_core_channels_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/core/channels/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_core_channels_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_core_channels_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_core_channel_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * DELETE ^/admin/core/channels/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_core_channels_detail(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_core_channels_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete
  ret = delete_core_channel_info(detail);
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
 * GET ^/admin/core/modules$ request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_core_modules(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_core_modules.");

  // get info
  j_tmp = get_core_modules_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_core_modules_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_core_modules_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_core_module_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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
 * POST ^/admin/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_post_admin_core_modules_detail(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_post_admin_core_modules_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // load module
  ret = load_module(detail);
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
 * PUT ^/admin/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_put_admin_core_modules_detail(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_put_admin_core_modules_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // reload module
  ret = reload_module(detail);
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
 * DELETE ^/admin/core/modules/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_delete_admin_core_modules_detail(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_delete_admin_core_modules_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // unload module
  ret = unload_module(detail);
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
 * GET ^/admin/core/systems$ request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_core_systems(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_core_systems.");

  // get info
  j_tmp = get_core_systems_all();
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * GET ^/admin/core/systems/(.*) request handler.
 * @param req
 * @param data
 */
void admin_htp_get_admin_core_systems_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired admin_htp_get_admin_core_systems_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail info
  j_tmp = get_core_system_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not find info.");
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




//////////////////////////////////////////////////////////



static json_t* get_user_users_all(void)
{
  json_t* j_res;

  j_res = user_get_userinfos_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool create_user_userinfo(const json_t* j_data)
{
  int ret;
  char* uuid;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  uuid = utils_gen_uuid();

  ret = user_create_userinfo(uuid, j_data);
  sfree(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create userinfo.");
    return false;
  }

  return true;
}

static json_t* get_user_userinfo(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = user_get_userinfo_info(uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool update_user_userinfo(const char* uuid, const json_t* j_data)
{
  int ret;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = user_update_userinfo_info(uuid, j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool delete_user_userinfo(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = user_delete_userinfo_info(uuid);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_user_contacts_all(void)
{
  json_t* j_res;

  j_res = user_get_contacts_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_user_contact_info(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = user_get_contact_info(uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool create_user_contact_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = user_create_contact_info(j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create userinfo.");
    return false;
  }

  return true;
}

static bool update_user_contact_info(const char* uuid, const json_t* j_data)
{
  int ret;

  if((uuid == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = user_update_contact_info(uuid, j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool delete_user_contact_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = user_delete_contact_info(uuid);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_user_permissions_all(void)
{
  json_t* j_res;

  j_res = user_get_permissions_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_user_permission_info(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = user_get_permission_info(uuid);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool create_user_permission_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = user_create_permission_info(j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create userinfo.");
    return false;
  }

  return true;
}

static bool delete_user_permission_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = user_delete_permission_info(uuid);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_queue_queues_all(void)
{
  json_t* j_res;

  j_res = queue_get_queue_params_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_queue_queue_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = queue_get_queue_param_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_queue_members_all(void)
{
  json_t* j_res;

  j_res = queue_get_members_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_queue_member_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = queue_get_member_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool create_queue_member_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = queue_action_add_member_to_queue(j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool update_queue_member_info(const char* key, const json_t* j_data)
{
  int ret;

  if((key == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  ret = queue_action_update_member_paused_penalty(j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool delete_queue_member_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = queue_action_delete_member_from_queue(key);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_queue_entries_all(void)
{
  json_t* j_res;

  j_res = queue_get_entries_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_queue_entry_info(const char* key)
{
  json_t* j_res;

  j_res = queue_get_entry_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool delete_queue_entry_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = call_hangup_by_unique_id(key);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_park_parkedcalls_all(void)
{
  json_t* j_res;

  j_res = park_get_parkedcalls_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_park_parkedcall_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = park_get_parkedcall_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool delete_park_parkedcall_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = call_hangup_by_unique_id(key);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_park_parkinglots_all(void)
{
  json_t* j_res;

  j_res = park_get_parkinglots_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_park_parkinglot_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = park_get_parkinglot_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_park_cfg_parkinglots_all(void)
{
  json_t* j_res;

  j_res = park_cfg_get_parkinglots_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_park_cfg_parkinglot_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = park_cfg_get_parkinglot_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool create_park_cfg_parkinglot_info(const json_t* j_data)
{
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = park_cfg_create_parkinglot_info(j_data);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool update_park_cfg_parkinglot_info(const char* name, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_deep_copy(j_data);
  json_object_set_new(j_tmp, "name", json_string(name));

  ret = park_cfg_update_parkinglot_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool delete_park_cfg_parkinglot_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = park_cfg_delete_parkinglot_info(name);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_core_channels_all(void)
{
  json_t* j_res;

  j_res = core_get_channels_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_core_channel_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = core_get_channel_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool delete_core_channel_info(const char* key)
{
  int ret;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = call_hangup_by_unique_id(key);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_core_modules_all(void)
{
  json_t* j_res;

  j_res = core_get_modules_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_core_module_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = core_get_module_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool load_module(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = core_module_load(name);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool reload_module(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = core_module_reload(name);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool unload_module(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = core_module_unload(name);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_core_systems_all(void)
{
  json_t* j_res;

  j_res = core_get_systems_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_core_system_info(const char* key)
{
  json_t* j_res;

  if(key == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = core_get_system_info(key);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_park_configurations_all(void)
{
  json_t* j_res;

  j_res = park_get_configurations_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_park_configuration_info(const char* name)
{
  json_t* j_res;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = park_get_configuration_info(name);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool update_park_configuration_info(const char* name, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;

  if((name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_deep_copy(j_data);
  json_object_set_new(j_tmp, "name", json_string(name));

  ret = park_update_configuration_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool delete_park_configuration_info(const char* name)
{
  int ret;

  if(name == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = park_delete_configuration_info(name);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Get given admin info.
 * @param authtoken
 * @return
 */
static json_t* get_admin_info(const json_t* j_user)
{
  json_t* j_res;
  const char* uuid_user;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_admin_info.");

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_NOTICE, "Could not get user uuid info.");
    return NULL;
  }

  // get user info
  j_res = user_get_userinfo_info(uuid_user);
  if(j_res == NULL) {
    slog(LOG_NOTICE, "Could not get user info.");
    return NULL;
  }

  // remove password object
  json_object_del(j_res, "password");

  return j_res;
}

/**
 * Update given admin info.
 * @param j_user
 * @param j_data
 * @return
 */
static bool update_admin_info(const json_t* j_user, const json_t* j_data)
{
  int ret;
  const char* uuid;

  if((j_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_admin_info.");

  uuid = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid == NULL) {
    slog(LOG_NOTICE, "Could not get user uuid info.");
    return false;
  }

  // update
  ret = user_update_userinfo_info(uuid, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update userinfo.");
    return false;
  }

  return true;
}

/**
 * Callback handler.
 * publish event.
 * admin.core.channel.<type>
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_core_db_channel(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  char* topic;
  int ret;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_core_db_channel.");

  // create event depends on type
  if(type == EN_RESOURCE_CREATE) {
    // set event type
    event_type = EN_PUBLISH_CREATE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else if(type == EN_RESOURCE_UPDATE) {
    // set event type
    event_type = EN_PUBLISH_UPDATE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else if(type == EN_RESOURCE_DELETE) {
    // set event type
    event_type = EN_PUBLISH_DELETE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else {
    // something was wrong
    slog(LOG_ERR, "Unsupported resource update type. type[%d]", type);
    return false;
  }

  // create topic
  asprintf(&topic, "%s", DEF_PUBLISH_TOPIC_PREFIX_ADMIN);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_ADMIN_CORE_CHANNEL, event_type, j_event);
  sfree(topic);
  json_decref(j_event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Callback handler.
 * publish event.
 * admin.core.module.<type>
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_core_db_module(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  char* topic;
  int ret;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_core_db_module.");

  // create event depends on type
  if(type == EN_RESOURCE_CREATE) {
    // set event type
    event_type = EN_PUBLISH_CREATE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else if(type == EN_RESOURCE_UPDATE) {
    // set event type
    event_type = EN_PUBLISH_UPDATE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else if(type == EN_RESOURCE_DELETE) {
    // set event type
    event_type = EN_PUBLISH_DELETE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else {
    // something was wrong
    slog(LOG_ERR, "Unsupported resource update type. type[%d]", type);
    return false;
  }

  // create topic
  asprintf(&topic, "%s", DEF_PUBLISH_TOPIC_PREFIX_ADMIN);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_ADMIN_CORE_MODULE, event_type, j_event);
  sfree(topic);
  json_decref(j_event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Callback handler.
 * publish event.
 * admin.park.parkedcall.<type>
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_park_db_parkedcall(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  char* topic;
  int ret;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_park_db_parkedcall.");

  // create event depends on type
  if(type == EN_RESOURCE_CREATE) {
    // set event type
    event_type = EN_PUBLISH_CREATE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else if(type == EN_RESOURCE_UPDATE) {
    // set event type
    event_type = EN_PUBLISH_UPDATE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else if(type == EN_RESOURCE_DELETE) {
    // set event type
    event_type = EN_PUBLISH_DELETE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else {
    // something was wrong
    slog(LOG_ERR, "Unsupported resource update type. type[%d]", type);
    return false;
  }

  // create topic
  asprintf(&topic, "%s", DEF_PUBLISH_TOPIC_PREFIX_ADMIN);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_ADMIN_PARK_PARKEDCALL, event_type, j_event);
  sfree(topic);
  json_decref(j_event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}
