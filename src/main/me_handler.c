/*
 * me_handler.c
 *
 *  Created on: Feb 4, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <jansson.h>
#include <string.h>

#include "slog.h"
#include "common.h"
#include "utils.h"

#include "http_handler.h"
#include "user_handler.h"
#include "resource_handler.h"
#include "sip_handler.h"
#include "pjsip_handler.h"

#include "me_handler.h"

static char* create_public_url(const char* target);
static json_t* create_contact_info(const char* id, const char* password);

static json_t* get_me_info(const char* authtoken);

static json_t* get_contact_info(const char* type, const char* target);
static json_t* get_contact_info_pjsip(const char* target);
static json_t* get_contact_info_sip(const char* target);

bool init_me_handler(void)
{
  slog(LOG_DEBUG, "Fired init_me_handler.");
  return true;
}

bool reload_me_handler(void)
{
  slog(LOG_DEBUG, "Fired reload_me_handler.");

  term_me_handler();
  init_me_handler();

  return true;
}

void term_me_handler(void)
{
  slog(LOG_DEBUG, "Fired term_handler.");
  return;
}


/**
 * GET ^/me/info request handler.
 * @param req
 * @param data
 */
void htp_get_me_info(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  char* token;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_me_info.");

  // check permission
  ret = http_is_request_has_permission(req, DEF_USER_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get token
  token = http_get_authtoken(req);
  if(token == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_me_info(token);
  sfree(token);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get me info.");
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
 * Get given authtoken's me info.
 * @param authtoken
 * @return
 */
static json_t* get_me_info(const char* authtoken)
{
  json_t* j_res;
  json_t* j_contacts;
  json_t* j_contact;
  json_t* j_tmp;
  int idx;
  const char* user_uuid;
  const char* contact_type;
  const char* contact_target;

  if(authtoken == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_me_info. authtoken[%s]", authtoken);

  // get user info
  j_res = get_user_userinfo_by_authtoken(authtoken);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not get user info. authtoken[%s]", authtoken);
    return NULL;
  }

  user_uuid = json_string_value(json_object_get(j_res, "uuid"));
  if(user_uuid == NULL) {
    slog(LOG_ERR, "Could not get user_uuid info.");
    json_decref(j_res);
    return NULL;
  }

  // get and update contacts info
  j_contacts = get_user_contacts_by_user_uuid(user_uuid);
  json_array_foreach(j_contacts, idx, j_contact) {
    contact_type = json_string_value(json_object_get(j_contact, "type"));
    if(contact_type == NULL) {
      slog(LOG_NOTICE, "Could not get contact type.");
      continue;
    }

    contact_target = json_string_value(json_object_get(j_contact, "target"));
    if(contact_target == NULL) {
      slog(LOG_NOTICE, "Could not get contact target.");
      continue;
    }

    // get contact info
    j_tmp = get_contact_info(contact_type, contact_target);
    if(j_tmp == NULL) {
      slog(LOG_NOTICE, "Could not get contact info. contact_type[%s], contact_target[%s]", contact_type, contact_target);
      continue;
    }

    json_object_set_new(j_contact, "info", j_tmp);
  }

  json_object_set_new(j_res, "contacts", j_contacts);

  return j_res;
}

/**
 * Get contact info of given data.
 * @param type
 * @param target
 * @return
 */
static json_t* get_contact_info(const char* type, const char* target)
{
  json_t* j_tmp;

  if((type == NULL) || (target == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_contact_info. type[%s], target[%s]", type, target);

  if(strcmp(type, DEF_USER_CONTACT_TYPE_ENDPOINT) == 0) {
    j_tmp = get_contact_info_pjsip(target);
  }
  else if(strcmp(type, DEF_USER_CONTACT_TYPE_PEER) == 0) {
    j_tmp = get_contact_info_sip(target);
  }
  else {
    slog(LOG_WARNING, "Wrong contact type. type[%s], target[%s]", type, target);
    j_tmp = NULL;
  }

  return j_tmp;
}

/**
 * Get contact info for pjsip type.
 * @param target
 * @return
 */
static json_t* get_contact_info_pjsip(const char* target)
{
  json_t* j_res;
  json_t* j_endpoint;
  json_t* j_auth;

  if(target == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_contact_info_pjsip. target[%s]", target);

  j_endpoint = get_pjsip_endpoint_info(target);
  if(j_endpoint == NULL) {
    slog(LOG_NOTICE, "Could not get pjsip endpoint info.");
    return NULL;
  }

  j_auth = get_pjsip_auth_info(json_string_value(json_object_get(j_endpoint, "auth")));
  if(j_auth == NULL) {
    slog(LOG_NOTICE, "Could not get pjsip auth info.");
    json_decref(j_endpoint);
    return NULL;
  }

  // create contact info
  j_res = create_contact_info(
      json_string_value(json_object_get(j_endpoint, "object_name")),
      json_string_value(json_object_get(j_auth, "password"))
      );
  json_decref(j_endpoint);
  json_decref(j_auth);

  return j_res;
}

/**
 * Get contact info for sip type.
 * @param target
 * @return
 */
static json_t* get_contact_info_sip(const char* target)
{
  json_t* j_account;
  json_t* j_res;

  if(target == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_contact_info_sip. target[%s]", target);

  // get peeraccount info
  j_account = get_sip_peeraccount_info(target);
  if(j_account == NULL) {
    slog(LOG_ERR, "Could not get peeraccount info. target[%s]", target);
    return NULL;
  }

  // create contact info
  j_res = create_contact_info(
      json_string_value(json_object_get(j_account, "peer")),
      json_string_value(json_object_get(j_account, "secret"))
      );
  json_decref(j_account);
  if(j_res == NULL) {
    slog(LOG_ERR, "Could not create contact info.");
    return NULL;
  }

  return j_res;
}

static char* create_public_url(const char* target)
{
  char* res;

  if(target == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&res, "sip:%s@%s",
      target,
      json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ast_serv_addr"))
      );

  return res;
}

static json_t* create_contact_info(const char* id, const char* password)
{
  json_t* j_res;
  char* pub_url;

  if((id == NULL) || (password == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired create_contact_info. id[%s], password[%s]", id, password);

  // create public url
  pub_url = create_public_url(id);
  if(pub_url == NULL) {
    slog(LOG_ERR, "Could not create public url. id[%s]", id);
    return NULL;
  }

  j_res = json_pack("{"
      "s:s, s:s, s:s, s:s"
      "}",

      "realm",        "localhost",
      "id",           id,
      "password",     password,
      "public_url",   pub_url
      );
  sfree(pub_url);

  return j_res;

}
