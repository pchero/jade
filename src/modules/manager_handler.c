/*
 * manager_handler.c
 *
 *  Created on: Apr 20, 2018
 *      Author: pchero
 */

#include <evhtp.h>

#include "slog.h"
#include "common.h"
#include "utils.h"

#include "http_handler.h"
#include "user_handler.h"

#include "manager_handler.h"

#define DEF_MANAGER_AUTHTOKEN_TYPE       "manager"

static json_t* get_users_all(const json_t* j_user);



bool manager_init_handler(void)
{
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
 * GET ^/manager/users request handler.
 * @param req
 * @param data
 */
void manager_htp_get_manager_users(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired manager_htp_get_manager_users.");

  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    slog(LOG_NOTICE, "Could not get correct userinfo.");
    http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_users_all(j_user);
  json_decref(j_user);
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

static json_t* get_users_all(const json_t* j_user)
{
  json_t* j_res;

  j_res = user_get_userinfos_all();
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static bool create_user(const json_t* j_data)
{
  int ret;

  // create pjsip


  // create user



  return true;
}
