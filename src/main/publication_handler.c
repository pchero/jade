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

#include <publication_handler.h>

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
  zmq_publish_message(topic, j_pub);
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
bool publication_publish_event_queue_member(const char* type, json_t* j_data)
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
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/queue/statuses/%s", tmp);
  sfree(tmp);

  // create event name
  asprintf(&event, "queue.member.%s", type);

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
bool publication_publish_event_queue_queue(const char* type, json_t* j_data)
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
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  tmp = utils_uri_encode(tmp_const);
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
bool publication_publish_event_queue_entry(const char* type, json_t* j_data)
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
  tmp = utils_uri_encode(tmp_const);
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
bool publication_publish_event_core_channel(const char* type, json_t* j_data)
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
  tmp = utils_uri_encode(tmp_const);
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
bool publication_publish_event_park_parkedcall(const char* type, json_t* j_data)
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
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/park/statuses/%s", tmp? : "");
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
 * park.parkinglot.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_park_parkinglot(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_park_parkinglot.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/park/statuses/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "park.parkinglot.%s", type);

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
bool publication_publish_event_core_agi(const char* type, json_t* j_data)
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
  tmp = utils_uri_encode(tmp_const);
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

/**
 * Publish event.
 * core.module.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_core_module(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_core_module.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "name"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/core/modules/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "core.module.%s", type);

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
 * pjsip.aor.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_pjsip_aor(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_pjsip_aor.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/pjsip/aors/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "pjsip.aor.%s", type);

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
 * pjsip.auth.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_pjsip_auth(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_pjsip_auth.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/pjsip/auths/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "pjsip.auth.%s", type);

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
 * pjsip.contact.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_pjsip_contact(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_pjsip_contact.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "id"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/pjsip/contacts/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "pjsip.contact.%s", type);

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
 * pjsip.endpoint.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_pjsip_endpoint(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_pjsip_endpoint.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "object_name"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/pjsip/endpoints/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "pjsip.endpoint.%s", type);

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
 * sip.peer.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_sip_peer(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_sip_peer.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "peer"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/sip/statuses/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "sip.peer.%s", type);

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
 * sip.registry.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_sip_registry(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_sip_registry.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "account"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/sip/statuses/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "sip.registry.%s", type);

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
 * dp.dpma.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_dp_dpma(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_dp_dpma.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "id"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/dp/statuses/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "dp.dpma.%s", type);

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
 * dp.dialplan.<type>
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event_dp_dialplan(const char* type, json_t* j_data)
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
  slog(LOG_DEBUG, "Fired publish_event_dp_dialplan.");

  // create topic
  tmp_const = json_string_value(json_object_get(j_data, "id"));
  tmp = utils_uri_encode(tmp_const);
  asprintf(&topic, "/dp/statuses/%s", tmp? : "");
  sfree(tmp);

  // create event name
  asprintf(&event, "dp.dialplan.%s", type);

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
 * Publish the event data with given parameters.
 * @param topic
 * @param event_prefix
 * @param type
 * @param j_data
 * @return
 */
bool publication_publish_event(const char* topic, const char* event_prefix, enum EN_PUBLISH_TYPES type, json_t* j_data)
{
  char* event;
  int ret;

  if((topic == NULL) || (event_prefix == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // create event name
  if(type == EN_PUBLISH_CREATE) {
    asprintf(&event, "%s.%s", event_prefix, DEF_PUB_TYPE_CREATE);
  }
  else if(type == EN_PUBLISH_UPDATE) {
    asprintf(&event, "%s.%s", event_prefix, DEF_PUB_TYPE_UPDATE);
  }
  else if(type == EN_PUBLISH_DELETE) {
    asprintf(&event, "%s.%s", event_prefix, DEF_PUB_TYPE_DELETE);
  }
  else {
    slog(LOG_ERR, "Could not get correct publish type. type[%u]", type);
    return false;
  }

  // publish event
  ret = publish_event(topic, event, j_data);
  sfree(event);
  if(ret == false) {
    slog(LOG_ERR, "Could not publish event.");
    return false;
  }

  return true;
}
