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
#include "chat_handler.h"
#include "publication_handler.h"
#include "call_handler.h"

#include "me_handler.h"

#define DEF_ME_CHAT_MESSAGE_COUNT   30

static char* create_public_url(const char* target);

static json_t* get_contacts_info(const json_t* j_user);

static json_t* get_userinfo(evhtp_request_t *req);

static json_t* get_contact_info(const json_t* j_user_contact);
static json_t* get_contact_info_pjsip(const char* target);

static json_t* get_me_info(const json_t* j_user);

static json_t* get_chats_info(const json_t* j_user);
static json_t* get_chatroom_info(const json_t* j_user, const char* uuid_userroom);
static bool create_chatroom_info(json_t* j_user, json_t* j_data);
static bool update_chatroom_info(json_t* j_user, const char* uuid_userroom, json_t* j_data);
static bool delete_chatroom_info(json_t* j_user, const char* uuid_userroom);

static json_t* get_chatmessages_info(json_t* j_user, const char* uuid_userroom, const char* timestamp, int count);
static bool create_chatmessage_info(json_t* j_user, const char* uuid_userroom, json_t* j_data);

static bool create_buddy_info(const json_t* j_user, const json_t* j_data);
static json_t* get_buddy_info(const json_t* j_user, const char* uuid_buddy);
static json_t* get_buddies_info(const json_t* j_user);
static bool update_buddy_info(const json_t* j_user, const char* detail, const json_t* j_data);
static bool delete_buddy_info(const json_t* j_user, const char* detail);

static json_t* get_calls_info(const json_t* j_user);
static bool create_call_info(const json_t* j_user, const json_t* j_data);
static bool create_call_to_user(const json_t* j_user, const json_t* j_data);
static char* get_callable_contact_from_useruuid(const char* uuid_user);

static json_t* get_search_info(json_t* j_user, const char* filter, const char* type);
static json_t* get_users_info_by_username(const char* username);


bool me_init_handler(void)
{
  slog(LOG_DEBUG, "Fired init_me_handler.");
  return true;
}

bool me_reload_handler(void)
{
  slog(LOG_DEBUG, "Fired reload_me_handler.");

  me_term_handler();
  me_init_handler();

  return true;
}

void me_term_handler(void)
{
  slog(LOG_DEBUG, "Fired term_handler.");
  return;
}


/**
 * GET ^/me/info request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_info(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_me_info.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_me_info(j_user);
  json_decref(j_user);
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
 * GET ^/me/chats request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_chats(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_me_chats.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get chat rooms info
  j_tmp = get_chats_info(j_user);
  json_decref(j_user);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get me info.");
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
 * POST ^/me/chats request handler.
 * @param req
 * @param data
 */
void me_htp_post_me_chats(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  json_t* j_user;
  json_t* j_data;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_me_chats.");

  // get user info
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    json_decref(j_user);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create chatroom
  ret = create_chatroom_info(j_user, j_data);
  json_decref(j_user);
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
 * GET ^/me/chats/<detail> request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_chats_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_me_chats_detail.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get chat room info
  j_tmp = get_chatroom_info(j_user, detail);
  json_decref(j_user);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get me chat room info.");
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
 * htp request handler.
 * PUT ^/me/chats/<detail> request handler.
 * @param req
 * @param data
 */
void me_htp_put_me_chats_detail(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_put_me_chats_detail.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

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

  // update
  ret = update_chatroom_info(j_user, detail, j_data);
  json_decref(j_user);
  json_decref(j_data);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not update chatroom info.");
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
 * DELETE ^/me/chats/<detail> request handler.
 * @param req
 * @param data
 */
void me_htp_delete_me_chats_detail(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_me_chats_detail.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  ret = delete_chatroom_info(j_user, detail);
  json_decref(j_user);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not update chatroom info.");
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
 * GET ^/me/chats/<detail>/messages request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_chats_detail_messages(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_user;
  json_t* j_tmp;
  char* detail;
  char* tmp;
  char* timestamp;
  int count;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_me_chats_detail_messages.");

  // get user info
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get detail
  detail = http_get_parsed_detail_start(req);
  if(detail == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get timestamp
  tmp = http_get_parameter(req, "timestamp");
  if(tmp == NULL) {
    timestamp = utils_get_utc_timestamp();
  }
  else {
    timestamp = strdup(tmp);
  }
  sfree(tmp);

  // get count
  tmp = http_get_parameter(req, "count");
  if(tmp == NULL) {
    count = DEF_ME_CHAT_MESSAGE_COUNT;
  }
  else {
    count = atoi(tmp);
  }
  sfree(tmp);

  // get messages
  j_tmp = get_chatmessages_info(j_user, detail, timestamp, count);
  json_decref(j_user);
  sfree(detail);
  sfree(timestamp);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get chat message info.");
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
 * POST ^/me/chats/<detail>/messages request handler.
 * @param req
 * @param data
 */
void me_htp_post_me_chats_detail_messages(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  json_t* j_user;
  json_t* j_data;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_me_chats_detail_messages.");

  // get user info
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    json_decref(j_user);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get detail
  detail = http_get_parsed_detail_start(req);
  if(detail == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create chat message
  ret = create_chatmessage_info(j_user, detail, j_data);
  json_decref(j_user);
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

/**
 * GET ^/me/buddies request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_buddies(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_get_me_buddies.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get buddies info
  j_tmp = get_buddies_info(j_user);
  json_decref(j_user);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get buddy info.");
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
 * POST ^/me/buddies request handler.
 * @param req
 * @param data
 */
void me_htp_post_me_buddies(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  json_t* j_user;
  json_t* j_data;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_post_me_buddies.");

  // get user info
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    json_decref(j_user);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create buddy
  ret = create_buddy_info(j_user, j_data);
  json_decref(j_user);
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
 * GET ^/me/buddies/<detail> request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_buddies_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_get_me_buddies_detail.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get chat room info
  j_tmp = get_buddy_info(j_user, detail);
  json_decref(j_user);
  sfree(detail);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get me buddy info.");
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
 * htp request handler.
 * PUT ^/me/buddies/<detail> request handler.
 * @param req
 * @param data
 */
void me_htp_put_me_buddies_detail(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  json_t* j_data;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_put_me_buddies_detail.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

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

  // update
  ret = update_buddy_info(j_user, detail, j_data);
  json_decref(j_user);
  json_decref(j_data);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not update buddy info.");
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
 * DELETE ^/me/buddies/<detail> request handler.
 * @param req
 * @param data
 */
void me_htp_delete_me_buddies_detail(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  char* detail;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_delete_me_buddies_detail.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // delete info
  ret = delete_buddy_info(j_user, detail);
  json_decref(j_user);
  sfree(detail);
  if(ret == false) {
    slog(LOG_ERR, "Could not update buddy info.");
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
 * GET ^/me/calls request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_calls(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_get_me_calls.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get calls info
  j_tmp = get_calls_info(j_user);
  json_decref(j_user);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get buddy info.");
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
 * POST ^/me/calls request handler.
 * @param req
 * @param data
 */
void me_htp_post_me_calls(evhtp_request_t *req, void *data)
{
  int ret;
  json_t* j_res;
  json_t* j_user;
  json_t* j_data;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_post_me_calls.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create call
  ret = create_call_info(j_user, j_data);
  json_decref(j_user);
  json_decref(j_data);
  if(ret == false) {
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
 * GET ^/me/search request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_search(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user;
  char* filter;
  char* type;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_get_me_search.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get filter
  filter = http_get_parameter(req, "filter");
  if(filter == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get type
  type = http_get_parameter(req, "type");
  if(type == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    sfree(filter);
    return;
  }

  j_tmp = get_search_info(j_user, filter, type);
  json_decref(j_user);
  sfree(filter);
  sfree(type);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get search info.");
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
 * GET ^/me/contacts request handler.
 * @param req
 * @param data
 */
void me_htp_get_me_contacts(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_get_me_contacts.");

  // get userinfo
  j_user = get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get contacts info
  j_tmp = get_contacts_info(j_user);
  json_decref(j_user);
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
 * Get given authtoken's me info.
 * @param authtoken
 * @return
 */
static json_t* get_me_info(const json_t* j_user)
{
  json_t* j_res;
  json_t* j_contacts;
  json_t* j_chats;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_me_info.");

  // get contacts
  j_contacts = get_contacts_info(j_user);
  if(j_contacts == NULL) {
    slog(LOG_WARNING, "Could not get contacts info.");
    return NULL;
  }

  // get chats info
  j_chats = get_chats_info(j_user);
  if(j_chats == NULL) {
    slog(LOG_WARNING, "Could not get chats info.");
    json_decref(j_contacts);
    return NULL;
  }

  // create result
  j_res = json_deep_copy(j_user);
  json_object_set_new(j_res, "contacts", j_contacts);
  json_object_set_new(j_res, "chats", j_chats);

  // remove password object
  json_object_del(j_res, "password");

  return j_res;
}

/**
 * Get contact info of given data.
 * @param type
 * @param target
 * @return
 */
static json_t* get_contact_info(const json_t* j_user_contact)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* target;

  if(j_user_contact == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_contact_info.");

  target = json_string_value(json_object_get(j_user_contact, "target"));
  if(target == NULL) {
    return NULL;
  }

  // we support only pjsip
  j_tmp = get_contact_info_pjsip(target);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get contact info from pjsip.");
    return NULL;
  }

  j_res = json_deep_copy(j_user_contact);
  json_object_set_new(j_res, "info", j_tmp);

  return j_res;
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
  const char* id;
  const char* password;
  char* pub_url;

  if(target == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_contact_info_pjsip. target[%s]", target);

  j_endpoint = pjsip_get_endpoint_info(target);
  if(j_endpoint == NULL) {
    slog(LOG_NOTICE, "Could not get pjsip endpoint info.");
    return NULL;
  }

  j_auth = pjsip_get_auth_info(json_string_value(json_object_get(j_endpoint, "auth")));
  if(j_auth == NULL) {
    slog(LOG_NOTICE, "Could not get pjsip auth info.");
    json_decref(j_endpoint);
    return NULL;
  }

  id = json_string_value(json_object_get(j_endpoint, "object_name"));
  password = json_string_value(json_object_get(j_auth, "password"));

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
  json_decref(j_endpoint);
  json_decref(j_auth);

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

/**
 * Get userinfo of given request
 * @param req
 * @return
 */
static json_t* get_userinfo(evhtp_request_t *req)
{
  char* token;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get token
  token = http_get_authtoken(req);
  if(token == NULL) {
    return NULL;
  }

  // get user info
  j_res = user_get_userinfo_by_authtoken(token);
  sfree(token);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

static json_t* get_contacts_info(const json_t* j_user)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_user_contacts;
  json_t* j_user_contact;
  int idx;
  const char* uuid_user;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get user uuid
  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Could not get user uuid info.");
    return NULL;
  }

  // get contacts info
  j_user_contacts = user_get_contacts_by_user_uuid(uuid_user);
  if(j_user_contacts == NULL) {
    slog(LOG_WARNING, "Could not get contacts info. uuid_user[%s]", uuid_user);
    return NULL;
  }

  j_res = json_array();
  json_array_foreach(j_user_contacts, idx, j_user_contact) {

    j_tmp = get_contact_info(j_user_contact);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_user_contacts);

  return j_res;
}

/**
 * Get list of given user's chatroom(userroom)
 * @param j_user
 * @return
 */
static json_t* get_chats_info(const json_t* j_user)
{
  json_t* j_res;
  json_t* j_userrooms;
  json_t* j_userroom;
  json_t* j_tmp;
  const char* tmp_const;
  int idx;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  tmp_const = json_string_value(json_object_get(j_user, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get uuid info.");
    return NULL;
  }

  // get all userrooms of user.
  j_userrooms = chat_get_userrooms_by_useruuid(tmp_const);
  if(j_userrooms == NULL) {
    slog(LOG_ERR, "Could not get chat userrooms info.");
    return NULL;
  }

  // create result
  j_res = json_array();
  json_array_foreach(j_userrooms, idx, j_userroom) {
    tmp_const = json_string_value(json_object_get(j_userroom, "uuid"));
    if(tmp_const == NULL) {
      continue;
    }

    j_tmp = get_chatroom_info(j_user, tmp_const);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_userrooms);

  return j_res;
}

/**
 *
 * @param j_user
 * @return
 */
static json_t* get_chatroom_info(const json_t* j_user, const char* uuid_userroom)
{
  int ret;
  json_t* j_res;
  json_t* j_room;
  const char* uuid;
  const char* uuid_user;
  const char* uuid_room;

  if((j_user == NULL) || (uuid_userroom == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  uuid = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid == NULL) {
    slog(LOG_ERR, "Could not get uuid info.");
    return NULL;
  }

  // get chat userroom info
  j_res = chat_get_userroom(uuid_userroom);
  if(j_res == NULL) {
    slog(LOG_NOTICE, "Could not get userroom info.");
    return NULL;
  }

  // get owner uuid.
  uuid_user = json_string_value(json_object_get(j_res, "uuid_user"));
  if(uuid_user == NULL) {
    json_decref(j_res);
    slog(LOG_ERR, "Could not check permission.");
    return NULL;
  }

  // check permission
  ret = strcmp(uuid, uuid_user);
  if(ret != 0) {
    json_decref(j_res);
    slog(LOG_NOTICE, "The given user has no permission.");
    return NULL;
  }

  // get chat room uuid.
  uuid_room = json_string_value(json_object_get(j_res, "uuid_room"));
  if(uuid_user == NULL) {
    json_decref(j_res);
    slog(LOG_ERR, "Could not get chat_room uuid info.");
    return NULL;
  }

  j_room = chat_get_room(uuid_room);
  json_object_del(j_res, "uuid_room");
  json_object_del(j_res, "uuid_user");
  if(j_room == NULL) {
    slog(LOG_NOTICE, "Could not get chat room info.");
    return j_res;
  }

  json_object_del(j_room, "message_table");

  json_object_set_new(j_res, "room", j_room);

  return j_res;
}

/**
 * Create given user's chatroom(userroom)
 * @param j_user
 * @return
 */
static bool create_chatroom_info(json_t* j_user, json_t* j_data)
{
  int ret;
  char* uuid_room;
  const char* tmp_uuid_userroom;
  const char* tmp_uuid_user;
  const char* uuid_user;
  int idx;
  json_t* j_userrooms;
  json_t* j_userroom;
  json_t* j_tmp;
  json_t* j_tmp_user;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_ERR, "Could not get uuid info.");
    return NULL;
  }

  // create chatroom uuid
  uuid_room = utils_gen_uuid();
  if(uuid_room == NULL) {
    slog(LOG_ERR, "Could not create uuid for chat room.");
    return false;
  }

  // create chat room
  ret = chat_create_room_with_foreach_userroom(uuid_room, uuid_user, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not create chat room info.");
    sfree(uuid_room);
    return false;
  }

  // get all userrooms
  j_userrooms = chat_get_userrooms_by_roomuuid(uuid_room);
  sfree(uuid_room);
  if(j_userrooms == NULL) {
    slog(LOG_ERR, "Could not get created userrooms info.");
    return false;
  }

  // publish event foreach
  json_array_foreach(j_userrooms, idx, j_userroom) {
    tmp_uuid_user = json_string_value(json_object_get(j_userroom, "uuid_user"));
    tmp_uuid_userroom = json_string_value(json_object_get(j_userroom, "uuid"));

    j_tmp_user = user_get_userinfo_info(tmp_uuid_user);
    j_tmp = get_chatroom_info(j_tmp_user, tmp_uuid_userroom);
    json_decref(j_tmp_user);
    if(j_tmp == NULL) {
      slog(LOG_NOTICE, "Could not create chatroom info for event publish.");
      continue;
    }

    // publish event
    ret = publication_publish_event_me_chat_room(EN_PUBLISH_CREATE, tmp_uuid_user, j_tmp);
    json_decref(j_tmp);
    if(ret == false) {
      slog(LOG_ERR, "Could not publish event.");
      continue;
    }
  }
  json_decref(j_userrooms);

  return true;
}

static bool update_chatroom_info(json_t* j_user, const char* uuid_userroom, json_t* j_data)
{
  int ret;
  const char* uuid_user;
  json_t* j_tmp;

  if((j_user == NULL) || (uuid_userroom == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_ERR, "Could not get user uuid.");
    return false;
  }

  // check permission
  ret = chat_is_user_userroom_owned(uuid_user, uuid_userroom);
  if(ret == false) {
    slog(LOG_WARNING, "Could not pass the permission check. uuid_user[%s], uuid_userroom[%s]", uuid_user, uuid_userroom);
    return NULL;
  }

  // update userroom
  ret = chat_update_userroom(uuid_userroom, j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update chatroom info.");
    return false;
  }

  // get updated userroom info
  j_tmp = get_chatroom_info(j_user, uuid_userroom);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get updated userroom info.");
    return false;
  }

  // publish event
  ret = publication_publish_event_me_chat_room(EN_PUBLISH_UPDATE, uuid_user, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

static bool delete_chatroom_info(json_t* j_user, const char* uuid_userroom)
{
  int ret;
  const char* uuid_user;
  json_t* j_userroom;

  if((j_user == NULL) || (uuid_userroom == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_ERR, "Could not get user uuid.");
    return false;
  }

  // check permission
  ret = chat_is_user_userroom_owned(uuid_user, uuid_userroom);
  if(ret == false) {
    slog(LOG_WARNING, "Could not pass the permission check. uuid_user[%s], uuid_userroom[%s]", uuid_user, uuid_userroom);
    return NULL;
  }

  // get chatroom info
  j_userroom = get_chatroom_info(j_user, uuid_userroom);
  if(j_userroom == NULL) {
    slog(LOG_WARNING, "Could not get delete userroom info.");
    return false;
  }

  ret = chat_delete_userroom(uuid_userroom);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete chatroom.");
    json_decref(j_userroom);
    return false;
  }

  // publish event
  ret = publication_publish_event_me_chat_room(EN_PUBLISH_DELETE, uuid_user, j_userroom);
  json_decref(j_userroom);
  if(ret == false) {
    return false;
  }

  return true;
}

static json_t* get_chatmessages_info(json_t* j_user, const char* uuid_userroom, const char* timestamp, int count)
{
  int ret;
  json_t* j_res;
  const char* uuid_user;

  if((j_user == NULL) || (uuid_userroom == NULL) || (timestamp == NULL) || (count < 0)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get uuid_user
  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Could not get uuid_user info.");
    return NULL;
  }

  // check permission
  ret = chat_is_user_userroom_owned(uuid_user, uuid_userroom);
  if(ret == false) {
    slog(LOG_WARNING, "Could not pass the permission check. uuid_user[%s], uuid_userroom[%s]", uuid_user, uuid_userroom);
    return NULL;
  }

  // get messages
  j_res = chat_get_userroom_messages_newest(uuid_userroom, timestamp, count);
  if(j_res == NULL) {
    slog(LOG_WARNING, "Could not get userroom messages info.");
    return NULL;
  }

  return j_res;
}

static bool create_chatmessage_info(json_t* j_user, const char* uuid_userroom, json_t* j_data)
{
  int ret;
  const char* uuid_user;
  char* uuid_message;
  char* uuid_room;
  json_t* j_message;

  if((j_user == NULL) || (uuid_userroom == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get uuid_user
  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_ERR, "Could not get user uuid info.");
    return false;
  }

  // permission check
  ret = chat_is_user_userroom_owned(uuid_user, uuid_userroom);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not pass the permissino check.");
    return false;
  }

  // create uuid_message
  uuid_message = utils_gen_uuid();
  if(uuid_message == NULL) {
    slog(LOG_ERR, "Could not create message uuid.");
    return false;
  }

  // create message
  ret = chat_create_message_to_userroom(uuid_message, uuid_userroom, uuid_user, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create chat message.");
    sfree(uuid_message);
    return false;
  }

  // get created message
  j_message = chat_get_userroom_message(uuid_message, uuid_userroom);
  sfree(uuid_message);
  if(j_message == NULL) {
    slog(LOG_ERR, "Could not get created message. uuid_userroom[%s]", uuid_userroom);
    return false;
  }

  // get room uuid
  uuid_room = chat_get_uuidroom_by_uuiduserroom(uuid_userroom);
  if(uuid_room == NULL) {
    slog(LOG_ERR, "Could not get room uuid info.");
    json_decref(j_message);
    return false;
  }

  // add uuid_room
  json_object_set_new(j_message, "uuid_room", json_string(uuid_room));

  // publish event
  ret = publication_publish_event_me_chat_message(EN_PUBLISH_CREATE, uuid_room, j_message);
  json_decref(j_message);
  sfree(uuid_room);
  if(ret == false) {
    slog(LOG_WARNING, "Could not publish chat message notification. ");
    return false;
  }

  return true;
}

static json_t* get_buddy_info(const json_t* j_user, const char* uuid_buddy)
{
  int ret;
  json_t* j_res;
  const char* uuid_user;

  if((j_user == NULL) || (uuid_buddy == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Could not get user uuid.");
    return NULL;
  }

  // check permission
  ret = user_is_user_owned_buddy(uuid_user, uuid_buddy);
  if(ret == false) {
    slog(LOG_WARNING, "The user does not have permission.");
    return false;
  }

  // get info
  j_res = user_get_buddy_info(uuid_buddy);
  if(j_res == NULL) {
    return NULL;
  }

  json_object_del(j_res, "uuid_owner");
  return j_res;
}

/**
 * Returns json array of given user's buddies
 * @param j_user
 * @return
 */
static json_t* get_buddies_info(const json_t* j_user)
{
  const char* uuid_user;
  json_t* j_buddies;
  json_t* j_buddy;
  json_t* j_res;
  json_t* j_tmp;
  const char* uuid;
  int idx;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Could not get user uuid info.");
    return NULL;
  }

  j_buddies = user_get_buddies_info_by_owneruuid(uuid_user);
  if(j_buddies == NULL) {
    slog(LOG_WARNING, "Could not get buddies info. user_uuid[%s]", uuid_user);
    return NULL;
  }

  j_res = json_array();
  json_array_foreach(j_buddies, idx, j_buddy) {
    uuid = json_string_value(json_object_get(j_buddy, "uuid"));
    if(uuid == NULL) {
      continue;
    }

    j_tmp = get_buddy_info(j_user, uuid);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_buddies);

  return j_res;
}

static bool create_buddy_info(const json_t* j_user, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* uuid_owner;
  char* uuid;

  if((j_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  uuid_owner = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_owner == NULL) {
    slog(LOG_WARNING, "Could not get user uuid info.");
    return false;
  }

  // create request data
  j_tmp = json_deep_copy(j_data);
  json_object_set_new(j_tmp, "uuid_owner", json_string(uuid_owner));

  // create
  uuid = utils_gen_uuid();
  ret = user_create_buddy_info(uuid, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    sfree(uuid);
    return false;
  }

  // get created buddy info
  j_tmp = user_get_buddy_info(uuid);
  sfree(uuid);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get created buddy info.");
    return false;
  }

  json_object_del(j_tmp, "uuid_owner");

  // publish event
  ret = publication_publish_event_me_buddy(EN_PUBLISH_CREATE, uuid_owner, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_WARNING, "Could not publish the event.");
    return false;
  }

  return true;
}

static bool update_buddy_info(const json_t* j_user, const char* detail, const json_t* j_data)
{
  int ret;
  json_t* j_tmp;
  const char* uuid_owner;

  if((j_user == NULL) || (detail == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  uuid_owner = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_owner == NULL) {
    slog(LOG_WARNING, "Could not get user uuid info.");
    return false;
  }

  // permission check
  ret = user_is_user_owned_buddy(uuid_owner, detail);
  if(ret == false) {
    slog(LOG_WARNING, "The user does not have permission.");
    return false;
  }

  // create request data
  j_tmp = json_deep_copy(j_data);
  json_object_set_new(j_tmp, "uuid_owner", json_string(uuid_owner));
  json_object_set_new(j_tmp, "uuid", json_string(detail));

  // update
  ret = user_update_buddy_info(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  // get updated buddy info
  j_tmp = user_get_buddy_info(detail);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get updated buddy info.");
    return false;
  }

  json_object_del(j_tmp, "uuid_owner");

  // publish event
  ret = publication_publish_event_me_buddy(EN_PUBLISH_CREATE, uuid_owner, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_WARNING, "Could not publish the event.");
    return false;
  }

  return true;
}

static bool delete_buddy_info(const json_t* j_user, const char* detail)
{
  int ret;
  const char* uuid_owner;
  json_t* j_tmp;

  if((j_user == NULL) || (detail == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  if((j_user == NULL) || (detail == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  uuid_owner = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_owner == NULL) {
    slog(LOG_WARNING, "Could not get user uuid info.");
    return false;
  }

  // permission check
  ret = user_is_user_owned_buddy(uuid_owner, detail);
  if(ret == false) {
    slog(LOG_WARNING, "The user does not have permission.");
    return false;
  }

  // get deleted buddy info
  j_tmp = get_buddy_info(j_user, detail);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get deleted buddy info.");
    return false;
  }

  // delete
  ret = user_delete_buddy_info(detail);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete buddy info.");
    json_decref(j_tmp);
    return false;
  }

  // publish event
  ret = publication_publish_event_me_buddy(EN_PUBLISH_DELETE, uuid_owner, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Returns json array of given user's calls
 * @param j_user
 * @return
 */
static json_t* get_calls_info(const json_t* j_user)
{
  const char* uuid_user;
  json_t* j_res;
  json_t* j_contacts;
  json_t* j_contact;
  json_t* j_calls;
  json_t* j_call;
  const char* target;
  int idx;
  int idx_2;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Could not get user uuid info.");
    return NULL;
  }

  // get user's contacts
  j_contacts = user_get_contacts_by_user_uuid(uuid_user);
  if(j_contacts == NULL) {
    slog(LOG_WARNING, "Could not get user's contacts info.");
    return NULL;
  }

  // get all calls info of given user's all contacts.
  j_res = json_array();
  json_array_foreach(j_contacts, idx, j_contact) {
    // get target
    target = json_string_value(json_object_get(j_contact, "target"));
    if(target == NULL) {
      slog(LOG_WARNING, "Could not get target info.");
      continue;
    }

    // get call info
    j_calls = call_get_channels_by_devicename(target);
    if(j_calls == NULL) {
      slog(LOG_NOTICE, "Could not get channels info. target[%s]", target);
      continue;
    }

    // add to result array
    json_array_foreach(j_calls, idx_2, j_call) {
      json_array_append(j_res, j_call);
    }
    json_decref(j_calls);

  }
  json_decref(j_contacts);

  return j_res;
}

static char* get_callable_contact_from_useruuid(const char* uuid_user)
{
  json_t* j_contacts;
  json_t* j_contact;
  const char* tmp_const;
  char* res;

  if(uuid_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get user's contacts
  j_contacts = user_get_contacts_by_user_uuid(uuid_user);
  if(j_contacts == NULL) {
    slog(LOG_WARNING, "Could not get user's contacts info.");
    return NULL;
  }

  j_contact = json_array_get(j_contacts, 0);
  if(j_contact == NULL) {
    slog(LOG_NOTICE, "The given user has no contact.");
    json_decref(j_contacts);
    return NULL;
  }

  tmp_const = json_string_value(json_object_get(j_contact, "target"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get contact uuid info.");
    json_decref(j_contacts);
    return NULL;
  }

  res = strdup(tmp_const);
  json_decref(j_contacts);

  return res;
}

static bool create_call_info(const json_t* j_user, const json_t* j_data)
{
  int ret;
  const char* type;

  if((j_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_call_info.");

  type = json_string_value(json_object_get(j_data, "destination_type"));
  if(type == NULL) {
    slog(LOG_NOTICE, "Could not get destination type info.");
    return false;
  }

  if(strcmp(type, "user") == 0) {
    ret = create_call_to_user(j_user, j_data);
  }
  else {
    slog(LOG_NOTICE, "Not support yet.");
    ret = false;
  }

  if(ret == false) {
    return false;
  }

  return true;
}

static bool create_call_to_user(const json_t* j_user, const json_t* j_data)
{
  int ret;
  const char* tmp_const;
  char* source;
  char* destination;

  // get source
  tmp_const = json_string_value(json_object_get(j_user, "uuid"));
  source = get_callable_contact_from_useruuid(tmp_const);
  if(source == NULL) {
    slog(LOG_NOTICE, "Could not get source info.");
    return false;
  }

  // get destination
  tmp_const = json_string_value(json_object_get(j_data, "destination"));
  destination = get_callable_contact_from_useruuid(tmp_const);
  if(destination == NULL) {
    slog(LOG_NOTICE, "Could not get destination info.");
    sfree(source);
    return false;
  }

  ret = call_originate_call_to_device(source, destination);
  sfree(source);
  sfree(destination);
  if(ret == false) {
    slog(LOG_ERR, "Could not create call to user.");
    return false;
  }

  return true;
}

static json_t* get_search_info(json_t* j_user, const char* filter, const char* type)
{
  json_t* j_res;

  if((j_user == NULL) || (filter == NULL) || (type == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  if(strcmp(type, "username") == 0) {
    j_res = get_users_info_by_username(filter);
  }
  else {
    slog(LOG_NOTICE, "Not support search type. type[%s]", type);
    return NULL;
  }

  return j_res;
}

/**
 *
 * @param username
 * @return
 */
static json_t* get_users_info_by_username(const char* username)
{
  json_t* j_res;
  json_t* j_tmp;

  if(username == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_tmp = user_get_userinfo_info_by_username(username);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "No find userinfo.");
    return NULL;
  }

  json_object_del(j_tmp, "password");
  json_object_del(j_tmp, "tm_create");
  json_object_del(j_tmp, "tm_update");

  j_res = json_array();
  json_array_append_new(j_res, j_tmp);

  return j_res;
}

