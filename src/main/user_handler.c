/*
 * user_handler.c
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */


#include "slog.h"
#include "common.h"
#include "utils.h"

#include "http_handler.h"
#include "resource_handler.h"

#include "user_handler.h"

static char* create_authtoken(const char* username, const char* password);
static bool create_userinfo(json_t* j_data);


bool init_user_handler(void)
{
  json_t* j_tmp;

	slog(LOG_INFO, "Fired init_user_handler.");

	// create default admin
	j_tmp = json_pack("{s:s, s:s}",
	    "username", "admin",
	    "password", "admin"
	    );
	create_userinfo(j_tmp);
	json_decref(j_tmp);

	return true;
}

void term_user_handler(void)
{
	slog(LOG_INFO, "Fired term_user_handler.");
	return;
}

bool reload_user_handler(void)
{
	slog(LOG_INFO, "Fired reload_user");

	term_user_handler();
	init_user_handler();

	return true;
}

/**
 * htp request handler.
 * request: POST ^/user/login$
 * @param req
 * @param data
 */
void htp_post_user_login(evhtp_request_t *req, void *data)
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
  ret = get_htp_id_pass(req, &username, &password);
  if(ret == false) {
    sfree(username);
    sfree(password);
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create authtoken
  authtoken = create_authtoken(username, password);
  sfree(username);
  sfree(password);
  if(authtoken == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  j_tmp = json_pack("{s:s}",
      "authtoken",  authtoken
      );

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: DELETE ^/user/login$
 * @param req
 * @param data
 */
void htp_delete_user_login(evhtp_request_t *req, void *data)
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
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete authtoken
  ret = delete_user_authtoken_info(authtoken);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  j_res = create_default_result(EVHTP_RES_OK);
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

static char* create_authtoken(const char* username, const char* password)
{
  json_t* j_user;
  json_t* j_auth;
  char* token;
  char* timestamp;

  if((username == NULL) || (password == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get user info
  j_user = get_user_userinfo_info_by_username_pass(username, password);
  if(j_user == NULL) {
    slog(LOG_INFO, "Could not find correct userinfo of given data. username[%s], password[%s]", username, "*");
    return NULL;
  }

  // create
  token = gen_uuid();
  timestamp = get_utc_timestamp();
  j_auth = json_pack("{s:s, s:s, s:s}",
      "uuid",       token,
      "user_uuid",  json_string_value(json_object_get(j_user, "uuid")),

      "tm_create",  timestamp
      );
  sfree(timestamp);
  json_decref(j_user);

  // create auth info
  create_user_authtoken_info(j_auth);
  json_decref(j_auth);

  return token;
}

static bool create_userinfo(json_t* j_data)
{
  char* timestamp;
  char* uuid;
  json_t* j_tmp;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_userinfo.");

  j_tmp = json_deep_copy(j_data);

  timestamp = get_utc_timestamp();
  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));
  json_object_set_new(j_tmp, "tm_create", json_string(timestamp));
  sfree(uuid);
  sfree(timestamp);

  ret = create_user_userinfo_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}
