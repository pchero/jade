/*
 * subscription_handler.c
 *
 *  Created on: Apr 15, 2018
 *      Author: pchero
 */

#include <jansson.h>
#include <zmq.h>

#include "slog.h"
#include "user_handler.h"
#include "me_handler.h"
#include "zmq_handler.h"
#include "websocket_handler.h"

#include "subscription_handler.h"

static bool subscribe_topic(void* zmq_sock, const char* topic);


bool subscription_init_handler(void)
{
  return true;
}

bool subscription_term_handler(void)
{
  return true;
}

bool subscription_reload_handler(void)
{
  int ret;

  ret = subscription_term_handler();
  if(ret == false) {
    return false;
  }

  ret = subscription_init_handler();
  if(ret == false) {
    return false;
  }

  return true;
}

bool subscription_subscribe_topics_client(const char* authtoken, void* zmq_sock)
{
  json_t* j_user;
  json_t* j_authtoken;
  json_t* j_topics;
  json_t* j_topic;
  int idx;
  int ret;
  const char* type;
  const char* topic;

  if((authtoken == NULL) || (zmq_sock == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get userinfo
  j_user = user_get_userinfo_by_authtoken(authtoken);
  if(j_user == false) {
    slog(LOG_NOTICE, "Could not get user info.");
    return false;
  }

  j_authtoken = user_get_authtoken_info(authtoken);
  if(j_authtoken == NULL) {
    slog(LOG_NOTICE, "Could not get authtoken info.");
    json_decref(j_user);
    return false;
  }

  // get login type
  type = json_string_value(json_object_get(j_authtoken, "type"));
  if(type == NULL) {
    slog(LOG_NOTICE, "Could not get type info.");
    json_decref(j_user);
    json_decref(j_authtoken);
    return false;
  }

  // get all subscribe-able topics
  if(strcmp(type, "admin") == 0) {
    j_topics = json_array();
    j_topic = json_string("/");
    json_array_append_new(j_topics, j_topic);
  }
  else if(strcmp(type, "me") == 0) {
    j_topics = me_get_subscribable_topics_all(j_user);
  }

  if(j_topics == NULL) {
    slog(LOG_NOTICE, "Could not get subscribe topics.");
    json_decref(j_user);
    json_decref(j_authtoken);
    return false;
  }

  json_array_foreach(j_topics, idx, j_topic) {
    topic = json_string_value(j_topic);

    ret = subscribe_topic(zmq_sock, topic);
    if(ret == false) {
      slog(LOG_ERR, "Could not subscribe topic. topic[%s]", topic);
      continue;
    }
  }
  json_decref(j_topics);
  json_decref(j_user);
  json_decref(j_authtoken);

  return true;
}

static bool subscribe_topic(void* zmq_sock, const char* topic)
{
  int ret;

  if((zmq_sock == NULL) || (topic == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired subscribe_topic. topic[%s]", topic);

  ret = zmq_setsockopt(zmq_sock, ZMQ_SUBSCRIBE, topic, strlen(topic));
  if(ret != 0) {
    slog(LOG_ERR, "Could not subscribe topic. topic[%s], err[%d:%s]", topic, errno, strerror(errno));
    return false;
  }

  return true;
}

bool subscription_subscribe_topic(const char* authtoken, const char* topic)
{
  int ret;
  void* sock;

  if((authtoken == NULL) || (topic == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired subscription_subscribe_topic. authtoken[%s], topic[%s]", authtoken, topic);

  sock = websocket_get_subscription_socket(authtoken);
  if(sock == NULL) {
    slog(LOG_NOTICE, "Could not get subscription socket.");
    return false;
  }

  // subscribe topic
  ret = subscribe_topic(sock, topic);
  if(ret == false) {
    return false;
  }

  return true;
}
