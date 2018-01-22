/*
 * publish_handler.c
 *
 *  Created on: Dec 22, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include "slog.h"
#include "zmq_handler.h"
#include "utils.h"

#include "publish_handler.h"

static bool publish_event(const char* topic, const char* event_name, json_t* j_data);

/**
 * Publish event
 * @param topic
 * @param event_name
 * @param j_data
 * @return
 */
static bool publish_event(const char* topic, const char* event_name, json_t* j_data)
{
  json_t* j_pub;

  if((topic == NULL) || (event_name == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // create pub
  j_pub = json_object();
  json_object_set(j_pub, event_name, j_data);

  // event publish
  publish_message(topic, j_pub);
  json_decref(j_pub);

  return true;
}

/**
 * Publish event.
 * queue.member.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publish_event_queue_member(const char* type, json_t* j_data)
{
  const char* tmp_const;
  char* tmp;
  char* topic;
  char* event;
  int ret;

  if((type == NULL) || (j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired publish_event_queue_member.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  tmp = uri_encode(tmp_const);
  asprintf(&topic, "/queue/statuses/%s", tmp);
  sfree(tmp);

  // create event name
  asprintf(&event, "queue.queue.%s", type);

  // publish event
  ret = publish_event(topic, event, j_data);
  sfree(topic);
  sfree(event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Publish event.
 * queue.queue.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publish_event_queue_queue(const char* type, json_t* j_data)
{
  const char* tmp_const;
  char* tmp;
  char* topic;
  char* event;
  int ret;

  if((type == NULL) || (j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired publish_event_queue_queue.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "queue_name"));
  tmp = uri_encode(tmp_const);
  asprintf(&topic, "/queue/statuses/%s", tmp);
  sfree(tmp);

  // create event name
  asprintf(&event, "queue.queue.%s", type);

  // publish event
  ret = publish_event(topic, event, j_data);
  sfree(topic);
  sfree(event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Publish event.
 * queue.entry.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publish_event_queue_entry(const char* type, json_t* j_data)
{
  const char* tmp_const;
  char* tmp;
  char* topic;
  char* event;
  int ret;

  if((type == NULL) || (j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired publish_event_queue_entry.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "queue_name"));
  tmp = uri_encode(tmp_const);
  asprintf(&topic, "/queue/statuses/%s", tmp);
  sfree(tmp);

  // create event name
  asprintf(&event, "queue.entry.%s", type);

  // publish event
  ret = publish_event(topic, event, j_data);
  sfree(topic);
  sfree(event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Publish event.
 * core.channel.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publish_event_core_channel(const char* type, json_t* j_data)
{
  const char* tmp_const;
  char* tmp;
  char* topic;
  char* event;
  int ret;

  if((type == NULL) || (j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired publish_event_core_channel.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "caller_id_name"));
  tmp = uri_encode(tmp_const);
  asprintf(&topic, "/core/channels/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "core.channel.%s", type);

  // publish event
  ret = publish_event(topic, event, j_data);
  sfree(topic);
  sfree(event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Publish event.
 * park.parkedcall.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publish_event_park_parkedcall(const char* type, json_t* j_data)
{
  const char* tmp_const;
  char* tmp;
  char* topic;
  char* event;
  int ret;

  if((type == NULL) || (j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired publish_event_park_parkedcall.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "parking_lot"));
  tmp = uri_encode(tmp_const);
  asprintf(&topic, "/park/parkinglots/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "park.parkedcall.%s", type);

  // publish event
  ret = publish_event(topic, event, j_data);
  sfree(topic);
  sfree(event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}

/**
 * Publish event.
 * core.agi.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publish_event_core_agi(const char* type, json_t* j_data)
{
  const char* tmp_const;
  char* tmp;
  char* topic;
  char* event;
  int ret;

  if((type == NULL) || (j_data == NULL)){
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired publish_event_core_agi.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "caller_id_name"));
  tmp = uri_encode(tmp_const);
  asprintf(&topic, "/core/agis/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "core.agi.%s", type);

  // publish event
  ret = publish_event(topic, event, j_data);
  sfree(topic);
  sfree(event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}
