/*
 * manager_handler.c
 *
 *  Created on: Apr 20, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <evhtp.h>

#include "slog.h"
#include "common.h"
#include "utils.h"

#include "http_handler.h"
#include "user_handler.h"
#include "pjsip_handler.h"
#include "chat_handler.h"
#include "publication_handler.h"

#include "manager_handler.h"

#define DEF_MANAGER_AUTHTOKEN_TYPE          "manager"

#define DEF_PUBLISH_TOPIC_PREFIX_MANAGER     "/manager"

#define DEF_PUB_EVENT_PREFIX_MANAGER_USER    "manager.user"             // topic: DEF_PUBLISH_TOPIC_PREFIX_ME_INFO

static json_t* get_users_all(void);
static json_t* get_user_info(const char* uuid_user);

static json_t* get_manager_info(const json_t* j_user);
static bool update_manager_info(const json_t* j_user, const json_t* j_data);

static bool create_user_info(const json_t* j_data);
static bool create_user_contact(const char* uuid_user, const char* target);
static bool create_user_permission(const char* uuid_user, const json_t* j_data);
static bool create_user_user(const char* uuid_user, const json_t* j_data);
static bool create_user_target(const char* target);

static bool delete_user_info(const char* uuid);

static bool update_user_info(const char* uuid_user, const json_t* j_data);
static bool update_user_permission(const char* uuid_user, const json_t* j_data);
static bool update_user_user(const char* uuid_user, const json_t* j_data);

static bool cb_resource_handler_user_userinfo(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);
static bool cb_resource_handler_user_permission(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);


bool manager_init_handler(void)
{

  // register callback
  user_register_callback_userinfo(&cb_resource_handler_user_userinfo);
  user_register_callback_permission(&cb_resource_handler_user_permission);

  return true;
}

bool manager_term_handler(void)
{
  return true;
}

bool manager_reload_handler(void)
{
  int ret;

  ret = manager_term_handler();
  if(ret == false) {
    return false;
  }

  ret = manager_init_handler();
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * POST ^/manager/login request handler.
 * @param req
 * @param data
 */
void manager_htp_post_manager_login(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired manager_htp_post_manager_login.");

  // get username/pass
  ret = http_get_htp_id_pass(req, &username, &password);
  if(ret == false) {
    sfree(username);
    sfree(password);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create authtoken
  authtoken = user_create_authtoken(username, password, DEF_MANAGER_AUTHTOKEN_TYPE);
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
 * GET ^/manager/info request handler.
 * @param req
 * @param data
 */
void manager_htp_get_manager_info(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_get_manager_info.");

  // get userinfo
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_manager_info(j_user);
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
 * PUT ^/manager/info request handler.
 * @param req
 * @param data
 */
void manager_htp_put_manager_info(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  json_t* j_data;
  json_t* j_user;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_put_manager_users.");

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
  ret = update_manager_info(j_user, j_data);
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
 * GET ^/manager/users request handler.
 * @param req
 * @param data
 */
void manager_htp_get_manager_users(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_get_manager_users.");

  // get info
  j_tmp = get_users_all();
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
 * POST ^/manager/users request handler.
 * @param req
 * @param data
 */
void manager_htp_post_manager_users(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  json_t* j_data;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_post_manager_users.");

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create info
  ret = create_user_info(j_data);
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
 * GET ^/manager/users/<detail> request handler.
 * @param req
 * @param data
 */
void manager_htp_get_manager_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_get_manager_users_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get user info
  j_tmp = get_user_info(detail);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get manager user info.");
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
 * DELETE ^/manager/users/<detail> request handler.
 * @param req
 * @param data
 */
void manager_htp_delete_manager_users_detail(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_delete_manager_users_detail.");

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete user info
  ret = delete_user_info(detail);
  sfree(detail);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete manager user info.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
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
 * PUT ^/manager/users/<detail> request handler.
 * @param req
 * @param data
 */
void manager_htp_put_manager_users_detail(evhtp_request_t *req, void *data)
{
  int ret;
  char* detail;
  json_t* j_res;
  json_t* j_data;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_put_manager_users.");

  // get detail
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    sfree(detail);
    return;
  }

  // update info
  ret = update_user_info(detail, j_data);
  json_decref(j_data);
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

static json_t* get_user_info(const char* uuid_user)
{
  json_t* j_res;
  json_t* j_perms;
  json_t* j_perm;
  int idx;

  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get user info
  j_res = user_get_userinfo_info(uuid_user);
  if(j_res == NULL) {
    slog(LOG_NOTICE, "Could not get user info.");
    return NULL;
  }

  // get permissions info
  j_perms = user_get_permissions_by_useruuid(uuid_user);
  if(j_perms != NULL) {

    json_array_foreach(j_perms, idx, j_perm) {
      json_object_del(j_perm, "user_uuid");
      json_object_del(j_perm, "uuid");
    }

    json_object_set_new(j_res, "permissions", j_perms);
  }

  return j_res;
}

/**
 * Get given manager info.
 * @param authtoken
 * @return
 */
static json_t* get_manager_info(const json_t* j_user)
{
  json_t* j_res;
  const char* uuid_user;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_info.");

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
 * Update given manager info.
 * @param j_user
 * @param j_data
 * @return
 */
static bool update_manager_info(const json_t* j_user, const json_t* j_data)
{
  int ret;
  const char* uuid;

  if((j_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_manager_info.");

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

static json_t* get_users_all(void)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_users;
  json_t* j_user;
  int idx;
  const char* uuid;

  j_users = user_get_userinfos_all();
  if(j_users == NULL) {
    return NULL;
  }

  j_res = json_array();
  json_array_foreach(j_users, idx, j_user) {
    uuid = json_string_value(json_object_get(j_user, "uuid"));

    j_tmp = get_user_info(uuid);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_users);

  return j_res;
}

static bool create_user_target(const char* target)
{
  int ret;

  if(target == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = pjsip_create_target_with_default_setting(target);
  if(ret == false) {
    slog(LOG_WARNING, "Could not create pjsip target info.");
    return false;
  }

  return true;
}

static bool create_user_user(const char* uuid_user, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* username;
  const char* password;
  const char* name;

  if((uuid_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get mandatory items
  username = json_string_value(json_object_get(j_data, "username"));
  password = json_string_value(json_object_get(j_data, "password"));
  name = json_string_value(json_object_get(j_data, "name"));
  if((username == NULL) || (password == NULL) || (name == NULL)) {
    slog(LOG_NOTICE, "Could not get mandatory items.");
    return false;
  }

  j_tmp = json_pack("{s:s, s:s, s:s}",
      "username",   username,
      "password",   password,
      "name",       name
      );
  ret = user_create_userinfo(uuid_user, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_WARNING, "Could not create userinfo.");
    return false;
  }

  return true;
}

static bool create_user_permission(const char* uuid_user, const json_t* j_data)
{
  int ret;
  int idx;
  json_t* j_tmp;
  json_t* j_permissions;
  json_t* j_permission;
  const char* permission;

  if((uuid_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_permissions = json_object_get(j_data, "permissions");
  if(j_permissions == NULL) {
    slog(LOG_NOTICE, "Could not get permissions info.");
    return false;
  }

  // create each permissions
  json_array_foreach(j_permissions, idx, j_permission) {
    permission = json_string_value(json_object_get(j_permission, "permission"));
    if(permission == NULL) {
      continue;
    }

    j_tmp = json_pack("{s:s, s:s}",
        "user_uuid",    uuid_user,
        "permission",   permission
        );
    ret = user_create_permission_info(j_tmp);
    json_decref(j_tmp);
    if(ret == false) {
      slog(LOG_WARNING, "Could not create permission info.");
    }
  }

  return true;
}

static bool create_user_contact(const char* uuid_user, const char* target)
{
  int ret;
  json_t* j_tmp;

  if((uuid_user == NULL) || (target == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_pack("{s:s, s:s}",
      "user_uuid",    uuid_user,
      "target",       target
      );

  ret = user_create_contact_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool create_user_info(const json_t* j_data)
{
  int ret;
  char* target;
  char* uuid_user;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // generate info
  target = utils_gen_uuid();
  uuid_user = utils_gen_uuid();

  // create target
  ret = create_user_target(target);
  if(ret == false) {
    slog(LOG_WARNING, "Could not create user target info.");
    pjsip_delete_target(target);
    sfree(target);
    sfree(uuid_user);
    return false;
  }

  // create user
  ret = create_user_user(uuid_user, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create user user info.");
    pjsip_delete_target(target);
    sfree(uuid_user);
    sfree(target);
    return false;
  }

  // create permission
  ret = create_user_permission(uuid_user, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create user permission info.");
    pjsip_delete_target(target);
    user_delete_userinfo_info(uuid_user);
    sfree(uuid_user);
    sfree(target);
    return false;
  }

  // create contact
  ret = create_user_contact(uuid_user, target);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create user contact info.");
    pjsip_delete_target(target);
    user_delete_userinfo_info(uuid_user);
    sfree(uuid_user);
    sfree(target);
    return false;
  }

  sfree(target);
  sfree(uuid_user);

  // reload pjsip config
  ret = pjsip_reload_config();
  if(ret == false) {
    slog(LOG_WARNING, "Could not reload pjsip_handler.");
    return false;
  }

  return true;
}

static bool delete_user_info(const char* uuid)
{
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // delete userinfo
  ret = user_delete_related_info_by_useruuid(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete user info. uuid[%s]", uuid);
    return false;
  }

  // delete chatinfo
  ret = chat_delete_info_by_useruuid(uuid);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete chat info.");
    return false;
  }

  return true;
}

static bool update_user_user(const char* uuid_user, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;

  if((uuid_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = json_pack("{s:s, s:s}",
      "name",       json_string_value(json_object_get(j_data, "name"))? : "",
      "password",   json_string_value(json_object_get(j_data, "password"))? : ""
      );

  ret = user_update_userinfo_info(uuid_user, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool update_user_permission(const char* uuid_user, const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_perms;
  json_t* j_perm;
  json_t* j_tmp;
  int idx;

  if((uuid_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_perms = json_object_get(j_data, "permissions");
  if(j_perms == NULL) {
    slog(LOG_NOTICE, "Could not get permissions info.");
    return false;
  }

  // delete all previous permissions
  ret = user_delete_permissions_by_useruuid(uuid_user);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not delete permission info.");
    return false;
  }

  // create new permissions
  json_array_foreach(j_perms, idx, j_perm) {
    tmp_const = json_string_value(json_object_get(j_perm, "permission"));

    j_tmp = json_pack("{s:s, s:s}",
        "user_uuid",    uuid_user,
        "permission",   tmp_const
        );

    ret = user_create_permission_info(j_tmp);
    json_decref(j_tmp);
    if(ret == false) {
      slog(LOG_NOTICE, "Could not create permission info.");
    }
  }

  return true;
}

static bool update_user_info(const char* uuid_user, const json_t* j_data)
{
  int ret;

  if((uuid_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // update userinfo
  ret = update_user_user(uuid_user, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update user info.");
    return false;
  }

  // update permission
  ret = update_user_permission(uuid_user, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update permission info.");
    return false;
  }

  return true;
}

/**
 * Returns all subscribable topics of manager module.
 * @param j_user
 * @return
 */
json_t* manager_get_subscribable_topics_all(const json_t* j_user)
{
  char* topic;
  json_t* j_res;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = json_array();

  // set topics
  asprintf(&topic, "%s", DEF_PUBLISH_TOPIC_PREFIX_MANAGER);
  json_array_append_new(j_res, json_string(topic));
  sfree(topic);

  return j_res;
}

/**
 * Callback handler for user_userinfo.
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_user_userinfo(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  char* topic;
  int ret;
  const char* tmp_const;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_handler_user_userinfo.");

  // create event depends on type
  if(type == EN_RESOURCE_CREATE) {
    // set event type
    event_type = EN_PUBLISH_CREATE;

    // get userinfo
    tmp_const = json_string_value(json_object_get(j_data, "uuid"));
    if(tmp_const == NULL) {
      slog(LOG_NOTICE, "Could not get user uuid.");
      return false;
    }

    // create event
    j_event = get_user_info(tmp_const);
    if(j_event == NULL) {
      slog(LOG_NOTICE, "Could not create event user info.");
      return false;
    }
  }
  else if(type == EN_RESOURCE_UPDATE) {
    // set event type
    event_type = EN_PUBLISH_UPDATE;

    // get userinfo
    tmp_const = json_string_value(json_object_get(j_data, "uuid"));
    if(tmp_const == NULL) {
      return false;
    }

    // create event
    j_event = get_user_info(tmp_const);
    if(j_event == NULL) {
      slog(LOG_NOTICE, "Could not create event user info.");
      return false;
    }
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
  asprintf(&topic, "%s", DEF_PUBLISH_TOPIC_PREFIX_MANAGER);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_MANAGER_USER, event_type, j_event);
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
 * manager.user.<type>
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_user_permission(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  char* topic;
  int ret;
  const char* uuid;
  json_t* j_tmp;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_user_permission.");

  // get userinfo
  uuid = json_string_value(json_object_get(j_data, "user_uuid"));
  if(uuid == NULL) {
    slog(LOG_NOTICE, "Could not get user_uuid info.");
    return false;
  }

  j_tmp = get_user_info(uuid);
  if(j_tmp == NULL) {
    // may already user removed.
    // skip the print log
    return false;
  }

  // create topic
  asprintf(&topic, "%s", DEF_PUBLISH_TOPIC_PREFIX_MANAGER);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_MANAGER_USER, EN_PUBLISH_UPDATE, j_tmp);
  sfree(topic);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}
