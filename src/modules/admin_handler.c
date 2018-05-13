/*
 * admin_handler.c
 *
 *  Created on: Apr 19, 2018
 *      Author: pchero
 */

#include <stdio.h>
#include <evhtp.h>
#include <jansson.h>

#include "slog.h"
#include "utils.h"

#include "user_handler.h"
#include "http_handler.h"

#include "admin_handler.h"

#define DEF_ADMIN_AUTHTOKEN_TYPE       "admin"


// user
static json_t* get_user_users_all(void);
static json_t* get_user_userinfo(const char* uuid);
static bool create_user_userinfo(const json_t* j_data);
static bool update_user_userinfo(const char* uuid, const json_t* j_data);
static bool delete_user_userinfo(const char* uuid);


bool admin_init_handler(void)
{
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
 * GET ^/admin/users request handler.
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
 * request: POST ^/admin/users$
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
