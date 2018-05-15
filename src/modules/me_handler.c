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
#include "subscription_handler.h"
#include "call_handler.h"

#include "me_handler.h"

#define DEF_ME_CHAT_MESSAGE_COUNT   30
#define DEF_ME_AUTHTOKEN_TYPE       "me"

#define DEF_PUBLISH_TOPIC_PREFIX_ME_INFO        "/me/info"
#define DEF_PUBLISH_TOPIC_PREFIX_ME_CHATROOM_MESSAGE    "/me/chats"

#define DEF_PUB_EVENT_PREFIX_ME_INFO                "me.info"             // topic: DEF_PUBLISH_TOPIC_PREFIX_ME_INFO
#define DEF_PUB_EVENT_PREFIX_ME_CHATROOM            "me.chats"            // topic: DEF_PUBLISH_TOPIC_PREFIX_ME_INFO
#define DEF_PUB_EVENT_PREFIX_ME_CHATROOM_MESSAGE    "me.chats.message"    // topic: DEF_PUBLISH_TOPIC_PREFIX_ME_CHATROOM
#define DEF_PUB_EVENT_PREFIX_ME_BUDDY               "me.buddies"          // topic: DEF_PUBLISH_TOPIC_PREFIX_ME_INFO

static char* create_public_url(const char* target);

static json_t* get_contacts_info(const json_t* j_user);

static json_t* get_contact_info(const json_t* j_user_contact);
static json_t* get_contact_info_pjsip(const char* target);

static json_t* get_me_info_by_user(const json_t* j_user);
static json_t* get_me_info(const char* uuid);
static bool update_me_info(const json_t* j_user, const json_t* j_data);

static json_t* get_room_info(const char* uuid_room);

static json_t* get_chats_info(const json_t* j_user);
static json_t* get_chatroom_info(const json_t* j_user, const char* uuid_userroom);
static json_t* get_chatroom_info_by_useruuid(const char* uuid_user, const char* uuid_userroom);
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

static bool add_subscription_to_useruuid(const char* uuid_user, const char* topic);
static bool add_subscription_to_useruuid_chatroom(const char* uuid_user, const char* uuid_room);

static bool remove_subscription_to_useruuid(const char* uuid_user, const char* topic);
static bool remove_subscription_to_useruuid_chatroom(const char* uuid_user, const char* uuid_room);

static bool cb_resource_handler_user_buddy(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);
static bool cb_resource_handler_user_userinfo(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);
static bool cb_resource_handler_chat_userroom(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);
static bool cb_resource_handler_chat_message(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data);


bool me_init_handler(void)
{
  slog(LOG_DEBUG, "Fired init_me_handler.");

  // register callback
  user_reigster_callback_db_buddy(&cb_resource_handler_user_buddy);
  user_register_callback_db_userinfo(&cb_resource_handler_user_userinfo);

  chat_register_callback_userroom(&cb_resource_handler_chat_userroom);
  chat_register_callback_message(cb_resource_handler_chat_message);

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
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_me_info_by_user(j_user);
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
 * htp request handler.
 * PUT ^/me/info request handler.
 * @param req
 * @param data
 */
void me_htp_put_me_info(evhtp_request_t *req, void *data)
{
  json_t* j_user;
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_put_me_info.");

  // get userinfo
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    json_decref(j_user);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update
  ret = update_me_info(j_user, j_data);
  json_decref(j_user);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update me info.");
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    json_decref(j_user);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    json_decref(j_user);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // detail parse
  detail = http_get_parsed_detail(req);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not get detail info.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    json_decref(j_user);
    return;
  }

  // get data
  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    sfree(detail);
    json_decref(j_user);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
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
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    sfree(detail);
    json_decref(j_user);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
  if(j_user == NULL) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  j_data = http_get_json_from_request_data(req);
  if(j_data == NULL) {
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    json_decref(j_user);
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
 * POST ^/me/login request handler.
 * @param req
 * @param data
 */
void me_htp_post_me_login(evhtp_request_t *req, void *data)
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
  slog(LOG_DEBUG, "Fired me_htp_post_me_login.");

  // get username/pass
  ret = http_get_htp_id_pass(req, &username, &password);
  if(ret == false) {
    sfree(username);
    sfree(password);
    http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create authtoken
  authtoken = user_create_authtoken(username, password, DEF_ME_AUTHTOKEN_TYPE);
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
void me_htp_delete_me_login(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* authtoken;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired me_htp_delete_me_login.");

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
  j_user = http_get_userinfo(req);
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
  j_user = http_get_userinfo(req);
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
 * Get given me info.
 * @param authtoken
 * @return
 */
static json_t* get_me_info_by_user(const json_t* j_user)
{
  json_t* j_res;
  const char* uuid_user;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_me_info.");

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_NOTICE, "Could not get user uuid info.");
    return NULL;
  }

  // get user info
  j_res = get_me_info(uuid_user);
  if(j_res == NULL) {
    slog(LOG_NOTICE, "Could not get user info.");
    return NULL;
  }

  return j_res;
}

static json_t* get_me_info(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get user info
  j_res = user_get_userinfo_info(uuid);
  if(j_res == NULL) {
    slog(LOG_NOTICE, "Could not get user info.");
    return NULL;
  }

  // remove password object
  json_object_del(j_res, "password");

  return j_res;
}

/**
 * Get given authtoken's me info.
 * @param authtoken
 * @return
 */
static bool update_me_info(const json_t* j_user, const json_t* j_data)
{
  int ret;
  const char* uuid_user;
//  json_t* j_tmp;

  if((j_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired update_me_info.");

  uuid_user = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid_user == NULL) {
    slog(LOG_NOTICE, "Could not get user uuid info.");
    return false;
  }

  // update userinfo
  ret = user_update_userinfo_info(uuid_user, j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update me info.");
    return false;
  }

  return true;
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

static json_t* get_room_info(const char* uuid_room)
{
  json_t* j_res;
  json_t* j_members;
  json_t* j_member;
  json_t* j_members_org;
  json_t* j_member_org;
  int idx;
  const char* uuid;
  json_t* j_user;

  if(uuid_room == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = chat_get_room(uuid_room);
  if(j_res == NULL) {
    slog(LOG_NOTICE, "Could not get chat room info.");
    return NULL;
  }

  j_members = json_array();
  j_members_org = json_object_get(j_res, "members");
  json_array_foreach(j_members_org, idx, j_member_org) {
    uuid = json_string_value(j_member_org);
    if(uuid == NULL) {
      continue;
    }

    j_user = user_get_userinfo_info(uuid);
    if(j_user == NULL) {
      continue;
    }

    j_member = json_pack("{s:s, s:s, s:s}",
        "uuid",       json_string_value(json_object_get(j_user, "uuid"))? : "",
        "username",   json_string_value(json_object_get(j_user, "username"))? : "",
        "name",       json_string_value(json_object_get(j_user, "name"))? : ""
        );
    json_decref(j_user);

    json_array_append_new(j_members, j_member);
  }

  json_object_set_new(j_res, "members", j_members);

  return j_res;
}

static json_t* get_chatroom_info_by_useruuid(const char* uuid_user, const char* uuid_userroom)
{
  json_t* j_res;
  json_t* j_room;
  const char* uuid_owner;
  const char* uuid_room;
  int ret;

  if((uuid_user == NULL) || (uuid_userroom == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get chat userroom info
  j_res = chat_get_userroom(uuid_userroom);
  if(j_res == NULL) {
    slog(LOG_NOTICE, "Could not get userroom info.");
    return NULL;
  }

  // get owner uuid.
  uuid_owner = json_string_value(json_object_get(j_res, "uuid_user"));
  if(uuid_user == NULL) {
    slog(LOG_ERR, "Could not check permission.");
    json_decref(j_res);
    return NULL;
  }

  // check permission
  ret = strcmp(uuid_owner, uuid_user);
  if(ret != 0) {
    slog(LOG_NOTICE, "The given user has no permission.");
    json_decref(j_res);
    return NULL;
  }

  // get chat room uuid.
  uuid_room = json_string_value(json_object_get(j_res, "uuid_room"));
  if(uuid_user == NULL) {
    slog(LOG_ERR, "Could not get chat_room uuid info.");
    json_decref(j_res);
    return NULL;
  }

  j_room = get_room_info(uuid_room);
  json_object_del(j_res, "uuid_room");
  json_object_del(j_res, "uuid_user");
  if(j_room == NULL) {
    slog(LOG_NOTICE, "Could not get chat room info.");
    json_decref(j_res);
    return NULL;
  }

  json_object_del(j_room, "message_table");

  json_object_set_new(j_res, "room", j_room);

  return j_res;
}

/**
 *
 * @param j_user
 * @return
 */
static json_t* get_chatroom_info(const json_t* j_user, const char* uuid_userroom)
{
  json_t* j_res;
  const char* uuid;

  if((j_user == NULL) || (uuid_userroom == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  uuid = json_string_value(json_object_get(j_user, "uuid"));
  if(uuid == NULL) {
    slog(LOG_ERR, "Could not get uuid info.");
    return NULL;
  }

  j_res = get_chatroom_info_by_useruuid(uuid, uuid_userroom);

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
  const char* uuid_user;

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
  sfree(uuid_room);
  if(ret == false) {
    slog(LOG_ERR, "Could not create chat room info.");
    return false;
  }

  return true;
}

static bool update_chatroom_info(json_t* j_user, const char* uuid_userroom, json_t* j_data)
{
  int ret;
  const char* uuid_user;

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

  return true;
}

static bool delete_chatroom_info(json_t* j_user, const char* uuid_userroom)
{
  int ret;
  const char* uuid_user;

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

  ret = chat_delete_userroom(uuid_userroom);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete chatroom.");
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

/**
 * Create chat message.
 * @param j_user
 * @param uuid_userroom
 * @param j_data
 * @return
 */
static bool create_chatmessage_info(json_t* j_user, const char* uuid_userroom, json_t* j_data)
{
  int ret;
  const char* uuid_user;
  char* uuid_message;

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
  sfree(uuid_message);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not create chat message.");
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
  sfree(uuid);
  json_decref(j_tmp);
  if(ret == false) {
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

  return true;
}

static bool delete_buddy_info(const json_t* j_user, const char* detail)
{
  int ret;
  const char* uuid_owner;

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

  // delete
  ret = user_delete_buddy_info(detail);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete buddy info.");
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
  slog(LOG_DEBUG, "Fired get_callable_contact_from_useruuid. uuid[%s]", uuid_user);

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

  if((j_user == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

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

/**
 * Returns all subscribable topics of me module
 * @param j_user
 * @return
 */
json_t* me_get_subscribable_topics_all(const json_t* j_user)
{
  char* topic;
  const char* tmp_const;
  json_t* j_res;
  json_t* j_chats;
  json_t* j_chat;
  int idx;

  if(j_user == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = json_array();

  // set topics for me info
  tmp_const = json_string_value(json_object_get(j_user, "uuid"));
  asprintf(&topic, "/me/info/%s", tmp_const);
  json_array_append_new(j_res, json_string(topic));
  sfree(topic);

  // get all involved chat rooms
  j_chats = chat_get_rooms_by_useruuid(tmp_const);
  if(j_chats == NULL) {
    slog(LOG_ERR, "Could not get chats info.");
    json_decref(j_res);
    return NULL;
  }

  // set topics for chat rooms
  json_array_foreach(j_chats, idx, j_chat) {
    tmp_const = json_string_value(json_object_get(j_chat, "uuid"));
    asprintf(&topic, "/me/chats/%s", tmp_const);
    json_array_append_new(j_res, json_string(topic));
    sfree(topic);
  }
  json_decref(j_chats);

  return j_res;
}

static bool add_subscription_to_useruuid_chatroom(const char* uuid_user, const char* uuid_room)
{
  int ret;
  char* topic;

  if((uuid_user == NULL) || (uuid_room == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // create topic
  asprintf(&topic, "%s/%s", DEF_PUBLISH_TOPIC_PREFIX_ME_CHATROOM_MESSAGE, uuid_room);

  ret = add_subscription_to_useruuid(uuid_user, topic);
  sfree(topic);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not subscribe the topic.");
    return false;
  }

  return true;
}

static bool remove_subscription_to_useruuid_chatroom(const char* uuid_user, const char* uuid_room)
{
  int ret;
  char* topic;

  if((uuid_user == NULL) || (uuid_room == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // create topic
  asprintf(&topic, "%s/%s", DEF_PUBLISH_TOPIC_PREFIX_ME_CHATROOM_MESSAGE, uuid_room);

  ret = remove_subscription_to_useruuid(uuid_user, topic);
  sfree(topic);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not unsubscribe the topic.");
    return false;
  }

  return true;
}

/**
 * Add the subscription to given topic of all given user's authtoken
 * @param uuid_user
 * @param topic
 * @return
 */
static bool add_subscription_to_useruuid(const char* uuid_user, const char* topic)
{
  int ret;
  int idx;
  json_t* j_tokens;
  json_t* j_token;
  const char* authtoken;

  if((uuid_user == NULL) || (topic == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired subscribe_created_chatroom. uuid_user[%s], topic[%s]", uuid_user, topic);

  // get all authtokens
  j_tokens = user_get_authtokens_user_type(uuid_user, DEF_ME_AUTHTOKEN_TYPE);
  if(j_tokens == NULL) {
    slog(LOG_ERR, "Could not get authtoken info.");
    return false;
  }

  json_array_foreach(j_tokens, idx, j_token) {
    authtoken = json_string_value(json_object_get(j_token, "uuid"));
    if(authtoken == NULL) {
      continue;
    }

    // add subscription
    ret = subscription_subscribe_topic(authtoken, topic);
    if(ret == false) {
      slog(LOG_NOTICE, "Could not add subscribe topic.");
      continue;
    }
  }
  json_decref(j_tokens);

  return true;
}

/**
 * Remove the subscription to given topic of all given user's authtoken
 * @param uuid_user
 * @param topic
 * @return
 */
static bool remove_subscription_to_useruuid(const char* uuid_user, const char* topic)
{
  int ret;
  int idx;
  json_t* j_tokens;
  json_t* j_token;
  const char* authtoken;

  if((uuid_user == NULL) || (topic == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired remove_subscription_to_useruuid. uuid_user[%s], topic[%s]", uuid_user, topic);

  // get all authtokens
  j_tokens = user_get_authtokens_user_type(uuid_user, DEF_ME_AUTHTOKEN_TYPE);
  if(j_tokens == NULL) {
    slog(LOG_ERR, "Could not get authtoken info.");
    return false;
  }

  json_array_foreach(j_tokens, idx, j_token) {
    authtoken = json_string_value(json_object_get(j_token, "uuid"));
    if(authtoken == NULL) {
      continue;
    }

    // remove subscription
    ret = subscription_unsubscribe_topic(authtoken, topic);
    if(ret == false) {
      slog(LOG_NOTICE, "Could not remove subscribe topic.");
      continue;
    }
  }
  json_decref(j_tokens);

  return true;
}

/**
 * Callback handler.
 * publish event.
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_user_buddy(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  char* topic;
  int ret;
  const char* uuid_owner;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_user_buddy.");

  uuid_owner = json_string_value(json_object_get(j_data, "uuid_owner"));
  if(uuid_owner == NULL) {
    slog(LOG_ERR, "Could not get owner uuid info.");
    return false;
  }

  if(type == EN_RESOURCE_CREATE) {
    event_type = EN_PUBLISH_CREATE;

    j_event = json_deep_copy(j_data);
    json_object_del(j_event, "uuid_owner");
  }
  else if(type == EN_RESOURCE_UPDATE) {
    event_type = EN_PUBLISH_UPDATE;

    j_event = json_deep_copy(j_data);
    json_object_del(j_event, "uuid_owner");
  }
  else if(type == EN_RESOURCE_DELETE) {
    event_type = EN_PUBLISH_DELETE;

    j_event = json_deep_copy(j_data);
    json_object_del(j_event, "uuid_owner");
  }
  else {
    slog(LOG_NOTICE, "Unsupported update type.");
    return false;
  }

  // create topic
  asprintf(&topic, "%s/%s", DEF_PUBLISH_TOPIC_PREFIX_ME_INFO, uuid_owner);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_ME_BUDDY, event_type, j_event);
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
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_user_userinfo(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  char* topic;
  int ret;
  const char* uuid;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_user_userinfo.");

  // get uuid
  uuid = json_string_value(json_object_get(j_data, "uuid"));
  if(uuid == NULL) {
    slog(LOG_ERR, "Could not get user uuid info.");
    return false;
  }

  if(type == EN_RESOURCE_UPDATE) {
    event_type = EN_PUBLISH_UPDATE;

    // create event
    j_event = get_me_info(uuid);
    if(j_event == NULL) {
      slog(LOG_NOTICE, "Could not get userinfo.");
      return false;
    }
  }
  else {
    slog(LOG_NOTICE, "Unsupported reource update type.");
    return false;
  }

  // create topic
  asprintf(&topic, "%s/%s", DEF_PUBLISH_TOPIC_PREFIX_ME_INFO, uuid);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_ME_INFO, event_type, j_event);
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
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_chat_userroom(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  int ret;
  char* topic;
  const char* uuid_user;
  const char* uuid_userroom;
  const char* uuid_room;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_chat_userroom.");


  uuid_userroom = json_string_value(json_object_get(j_data, "uuid"));
  uuid_user = json_string_value(json_object_get(j_data, "uuid_user"));
  uuid_room = json_string_value(json_object_get(j_data, "uuid_room"));

  if(type == EN_RESOURCE_CREATE) {
    event_type = EN_RESOURCE_CREATE;

    // add subscribe
    ret = add_subscription_to_useruuid_chatroom(uuid_user, uuid_room);
    if(ret == false) {
      slog(LOG_ERR, "Could not add subscription for created chatroom.");
      return false;
    }

    // create publish message
    j_event = get_chatroom_info_by_useruuid(uuid_user, uuid_userroom);
    if(j_event == NULL) {
      slog(LOG_NOTICE, "Could not create chatroom info for event publish.");
      return false;
    }
  }
  else if(type == EN_RESOURCE_UPDATE) {
    event_type = EN_RESOURCE_UPDATE;

    // create publish message
    j_event = get_chatroom_info_by_useruuid(uuid_user, uuid_userroom);
    if(j_event == NULL) {
      slog(LOG_NOTICE, "Could not create chatroom info for event publish.");
      return false;
    }
  }
  else if(type == EN_RESOURCE_DELETE) {
    event_type = EN_PUBLISH_DELETE;

    // delete subscribe
    ret = remove_subscription_to_useruuid_chatroom(uuid_user, uuid_room);
    if(ret == false) {
      slog(LOG_ERR, "Could not unsubscribe for deleted chatroom.");
      return false;
    }

    // create event
    j_event = json_deep_copy(j_data);
  }
  else {
    // should not get to here
    slog(LOG_ERR, "Unsupported resource update type. type[%d]", type);
    return false;
  }

  // create topic
  asprintf(&topic, "%s/%s", DEF_PUBLISH_TOPIC_PREFIX_ME_INFO, uuid_user);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_ME_CHATROOM, event_type, j_event);
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
 * @param type
 * @param j_data
 * @return
 */
static bool cb_resource_handler_chat_message(enum EN_RESOURCE_UPDATE_TYPES type, const json_t* j_data)
{
  int ret;
  char* topic;
  const char* uuid_room;
  json_t* j_event;
  enum EN_PUBLISH_TYPES event_type;

  if((j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired cb_resource_handler_chat_message.");

  uuid_room = json_string_value(json_object_get(j_data, "uuid_room"));
  if(uuid_room == NULL) {
    slog(LOG_NOTICE, "Could not get uuid_room info.");
    return false;
  }

  if(type == EN_RESOURCE_CREATE) {
    event_type = EN_PUBLISH_CREATE;

    // create event
    j_event = json_deep_copy(j_data);
  }
  else {
    // should not reach to here
    slog(LOG_INFO, "Unsupported reource update type. type[%d]", type);
    return false;
  }

  // create topic
  asprintf(&topic, "%s/%s", DEF_PUBLISH_TOPIC_PREFIX_ME_CHATROOM_MESSAGE, uuid_room);

  // publish event
  ret = publication_publish_event(topic, DEF_PUB_EVENT_PREFIX_ME_CHATROOM_MESSAGE, event_type, j_event);
  sfree(topic);
  json_decref(j_event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}


