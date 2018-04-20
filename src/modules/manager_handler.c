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
