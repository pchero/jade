/*
 * http_handler.c
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <jansson.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "http_handler.h"
#include "resource_handler.h"
#include "ob_http_handler.h"
#include "voicemail_handler.h"
#include "core_handler.h"
#include "agent_handler.h"
#include "sip_handler.h"
#include "pjsip_handler.h"
#include "queue_handler.h"
#include "park_handler.h"



#define API_VER "0.1"

extern app* g_app;
extern struct event_base* g_base;
evhtp_t* g_htp = NULL;

#define DEF_REG_MSGNAME "msg[0-9]{4}"

// ping
static void cb_htp_ping(evhtp_request_t *req, void *a);


// agent
static void cb_htp_agent_agents(evhtp_request_t *req, void *data);
static void cb_htp_agent_agents_detail(evhtp_request_t *req, void *data);


// core
static void cb_htp_core_channels(evhtp_request_t *req, void *data);
static void cb_htp_core_channels_detail(evhtp_request_t *req, void *data);
static void cb_htp_core_modules(evhtp_request_t *req, void *data);
static void cb_htp_core_modules_detail(evhtp_request_t *req, void *data);
static void cb_htp_core_systems(evhtp_request_t *req, void *data);
static void cb_htp_core_systems_detail(evhtp_request_t *req, void *data);


// sip
static void cb_htp_sip_peers(evhtp_request_t *req, void *data);
static void cb_htp_sip_peers_detail(evhtp_request_t *req, void *data);
static void cb_htp_sip_registries(evhtp_request_t *req, void *data);
static void cb_htp_sip_registries_detail(evhtp_request_t *req, void *data);


// pjsip
static void cb_htp_pjsip_endpoints(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_endpoints_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_aors(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_aors_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_auths(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_auths_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_contacts(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_contacts_detail(evhtp_request_t *req, void *data);


// queue
static void cb_htp_queue_entries(evhtp_request_t *req, void *data);
static void cb_htp_queue_entries_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_members(evhtp_request_t *req, void *data);
static void cb_htp_queue_members_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_queues(evhtp_request_t *req, void *data);
static void cb_htp_queue_queues_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_setting(evhtp_request_t *req, void *data);
static void cb_htp_queue_settings(evhtp_request_t *req, void *data);
static void cb_htp_queue_settings_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_statuses(evhtp_request_t *req, void *data);
static void cb_htp_queue_statuses_detail(evhtp_request_t *req, void *data);



// voicemail/
static void cb_htp_voicemail_setting(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_settings(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_settings_detail(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_users(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_users_detail(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_vms(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_vms_msgname(evhtp_request_t *req, void *data);


// databases
static void cb_htp_databases(evhtp_request_t *req, void *data);
static void cb_htp_databases_key(evhtp_request_t *req, void *data);


// device_states
static void cb_htp_device_states(evhtp_request_t *req, void *data);
static void cb_htp_device_states_detail(evhtp_request_t *req, void *data);

// parking_lots
static void cb_htp_park_parkinglots(evhtp_request_t *req, void *data);
static void cb_htp_park_parkinglots_detail(evhtp_request_t *req, void *data);

// parked_calls
static void cb_htp_park_parkedcalls(evhtp_request_t *req, void *data);
static void cb_htp_park_parkedcalls_detail(evhtp_request_t *req, void *data);




bool init_http_handler(void)
{
  const char* tmp_const;
  const char* http_addr;
  int http_port;
  int ret;

  g_htp = evhtp_new(g_base, NULL);

  http_addr = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "http_addr"));
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "http_port"));
  http_port = atoi(tmp_const);

  slog(LOG_INFO, "The http server info. addr[%s], port[%d]", http_addr, http_port);
  ret = evhtp_bind_socket(g_htp, http_addr, http_port, 1024);
  if(ret != 0) {
    slog(LOG_ERR, "Could not open the http server.");
    return false;
  }

  /////////////////////// apis


  //// ^/agent/
  // agents
  evhtp_set_regex_cb(g_htp, "^/agent/agents/(.*)", cb_htp_agent_agents_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/agent/agents$", cb_htp_agent_agents, NULL);



  //// ^/core/
  // channels
  evhtp_set_regex_cb(g_htp, "^/core/channels/(.*)", cb_htp_core_channels_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/core/channels$", cb_htp_core_channels, NULL);

  // modules
  evhtp_set_regex_cb(g_htp, "^/core/modules/(.*)", cb_htp_core_modules_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/core/modules$", cb_htp_core_modules, NULL);

  // systems
  evhtp_set_regex_cb(g_htp, "^/core/systems/(.*)", cb_htp_core_systems_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/core/systems$", cb_htp_core_systems, NULL);



  ////// ^/ob/
  ////// outbound modules
  // destinations
  evhtp_set_regex_cb(g_htp, "^/ob/destinations/("DEF_REG_UUID")", cb_htp_ob_destinations_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "^/ob/destinations$", cb_htp_ob_destinations, NULL);

  // plans
  evhtp_set_regex_cb(g_htp, "^/ob/plans/("DEF_REG_UUID")", cb_htp_ob_plans_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "^/ob/plans$", cb_htp_ob_plans, NULL);

  // campaigns
  evhtp_set_regex_cb(g_htp, "^/ob/campaigns/(/"DEF_REG_UUID")", cb_htp_ob_campaigns_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "^/ob/campaigns$", cb_htp_ob_campaigns, NULL);

  // dlmas
  evhtp_set_regex_cb(g_htp, "^/ob/dlmas/("DEF_REG_UUID")", cb_htp_ob_dlmas_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "^/ob/dlmas$", cb_htp_ob_dlmas, NULL);

  // dls
  evhtp_set_regex_cb(g_htp, "^/ob/dls/("DEF_REG_UUID")", cb_htp_ob_dls_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "^/ob/dls$", cb_htp_ob_dls, NULL);

  // dialings
  evhtp_set_regex_cb(g_htp, "^/ob/dialings/("DEF_REG_UUID")", cb_htp_ob_dialings_uuid, NULL);
  evhtp_set_regex_cb(g_htp, "^/ob/dialings$", cb_htp_ob_dialings, NULL);



  //// ^/park/
  // parking_lots
  evhtp_set_regex_cb(g_htp, "^/park/parkinglots/(.*)", cb_htp_park_parkinglots_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/park/parkinglots$", cb_htp_park_parkinglots, NULL);

  // parked_calls
  evhtp_set_regex_cb(g_htp, "^/park/parkedcalls/(.*)", cb_htp_park_parkedcalls_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/park/parkedcalls$", cb_htp_park_parkedcalls, NULL);



  //// ^/pjsip/
  // aors
  evhtp_set_regex_cb(g_htp, "^/pjsip/aors/(.*)", cb_htp_pjsip_aors_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/pjsip/aors$", cb_htp_pjsip_aors, NULL);

  // auths
  evhtp_set_regex_cb(g_htp, "^/pjsip/auths/(.*)", cb_htp_pjsip_auths_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/pjsip/auths$", cb_htp_pjsip_auths, NULL);

  // contacts
  evhtp_set_regex_cb(g_htp, "^/pjsip/contacts/(.*)", cb_htp_pjsip_contacts_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/pjsip/contacts$", cb_htp_pjsip_contacts, NULL);

  // endpoints
  evhtp_set_regex_cb(g_htp, "^/pjsip/endpoints/(.*)", cb_htp_pjsip_endpoints_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/pjsip/endpoints$", cb_htp_pjsip_endpoints, NULL);



  //// ^/queue/
  // entries
  evhtp_set_regex_cb(g_htp, "^/queue/entries/(.*)", cb_htp_queue_entries_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/queue/entries$", cb_htp_queue_entries, NULL);

  // members
  evhtp_set_regex_cb(g_htp, "^/queue/members/(.*)", cb_htp_queue_members_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/queue/members$", cb_htp_queue_members, NULL);

  // queues
  evhtp_set_regex_cb(g_htp, "^/queue/queues/(.*)", cb_htp_queue_queues_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/queue/queues$", cb_htp_queue_queues, NULL);

  // setting
  evhtp_set_regex_cb(g_htp, "^/queue/setting$", cb_htp_queue_setting, NULL);

  // settings
  evhtp_set_regex_cb(g_htp, "^/queue/settings/(.*)", cb_htp_queue_settings_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/queue/settings$", cb_htp_queue_settings, NULL);

  // statuses
  evhtp_set_regex_cb(g_htp, "^/queue/statuses/(.*)", cb_htp_queue_statuses_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/queue/statuses$", cb_htp_queue_statuses, NULL);



  //// ^/sip/
  // peers
  evhtp_set_regex_cb(g_htp, "^/sip/peers/(.*)", cb_htp_sip_peers_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/sip/peers$", cb_htp_sip_peers, NULL);

  // registries
  evhtp_set_regex_cb(g_htp, "^/sip/registries/(.*)", cb_htp_sip_registries_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/sip/registries$", cb_htp_sip_registries, NULL);



  //// ^/voicemail/
  // setting
  evhtp_set_regex_cb(g_htp, "^/voicemail/setting$", cb_htp_voicemail_setting, NULL);

  // settings
  evhtp_set_regex_cb(g_htp, "^/voicemail/settings/(.*)", cb_htp_voicemail_settings_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/voicemail/settings$", cb_htp_voicemail_settings, NULL);

  // users
  evhtp_set_regex_cb(g_htp, "^/voicemail/users/(.*)", cb_htp_voicemail_users_detail, NULL);
  evhtp_set_regex_cb(g_htp, "^/voicemail/users$", cb_htp_voicemail_users, NULL);

  // vms
  evhtp_set_regex_cb(g_htp, "^/voicemail/vms/("DEF_REG_MSGNAME")", cb_htp_voicemail_vms_msgname, NULL);
  evhtp_set_regex_cb(g_htp, "^/voicemail/vms$", cb_htp_voicemail_vms, NULL);




  // old apis.
  // will be deprecated.

  // register callback
  evhtp_set_cb(g_htp, "/ping", cb_htp_ping, NULL);

  // peers
  evhtp_set_cb(g_htp, "/peers/", cb_htp_sip_peers_detail, NULL);
  evhtp_set_cb(g_htp, "/peers", cb_htp_sip_peers, NULL);

  // databases - deprecated
  evhtp_set_cb(g_htp, "/databases/", cb_htp_databases_key, NULL);
  evhtp_set_cb(g_htp, "/databases", cb_htp_databases, NULL);

  // registres
  evhtp_set_cb(g_htp, "/registries/", cb_htp_sip_registries_detail, NULL);
  evhtp_set_cb(g_htp, "/registries", cb_htp_sip_registries, NULL);

  // queue_params
  evhtp_set_cb(g_htp, "/queue_params/", cb_htp_queue_queues_detail, NULL);
  evhtp_set_cb(g_htp, "/queue_params", cb_htp_queue_queues, NULL);

  // queue_members
  evhtp_set_cb(g_htp, "/queue_members/", cb_htp_queue_members_detail, NULL);
  evhtp_set_cb(g_htp, "/queue_members", cb_htp_queue_members, NULL);

  // queue_entries
  evhtp_set_cb(g_htp, "/queue_entries/", cb_htp_queue_entries_detail, NULL);
  evhtp_set_cb(g_htp, "/queue_entries", cb_htp_queue_entries, NULL);

  // queue_entries
  evhtp_set_cb(g_htp, "/channels/", cb_htp_core_channels_detail, NULL);
  evhtp_set_cb(g_htp, "/channels", cb_htp_core_channels, NULL);

  // agents
  evhtp_set_cb(g_htp, "/agents/", cb_htp_agent_agents_detail, NULL);
  evhtp_set_cb(g_htp, "/agents", cb_htp_agent_agents, NULL);

  // systems
  evhtp_set_cb(g_htp, "/systems/", cb_htp_core_systems_detail, NULL);
  evhtp_set_cb(g_htp, "/systems", cb_htp_core_systems, NULL);

  // device_states
  evhtp_set_cb(g_htp, "/device_states/", cb_htp_device_states_detail, NULL);
  evhtp_set_cb(g_htp, "/device_states", cb_htp_device_states, NULL);

  // parking_lots
  evhtp_set_cb(g_htp, "/parking_lots/", cb_htp_park_parkinglots_detail, NULL);
  evhtp_set_cb(g_htp, "/parking_lots", cb_htp_park_parkinglots, NULL);

  // parked_calls
  evhtp_set_cb(g_htp, "/parked_calls/", cb_htp_park_parkedcalls_detail, NULL);
  evhtp_set_cb(g_htp, "/parked_calls", cb_htp_park_parkedcalls, NULL);


  return true;
}

void term_http_handler(void)
{
  slog(LOG_INFO, "Terminate http handler.");
  if(g_htp != NULL) {
    evhtp_unbind_socket(g_htp);
    evhtp_free(g_htp);
  }
}

void simple_response_normal(evhtp_request_t *req, json_t* j_msg)
{
  char* res;

  if((req == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired simple_response_normal.");

  // add default headers
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Headers", "x-requested-with, content-type, accept, origin, authorization", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Origin", "*", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Max-Age", "86400", 1, 1));

  res = json_dumps(j_msg, JSON_ENCODE_ANY);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, EVHTP_RES_OK);
  sfree(res);

  return;
}

void simple_response_error(evhtp_request_t *req, int status_code, int err_code, const char* err_msg)
{
  char* res;
  json_t* j_tmp;
  json_t* j_res;
  int code;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired simple_response_error.");

  // add default headers
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Headers", "x-requested-with, content-type, accept, origin, authorization", 1, 1));
  evhtp_headers_add_header (req->headers_out, evhtp_header_new("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Origin", "*", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Max-Age", "86400", 1, 1));

  if(evhtp_request_get_method(req) == htp_method_OPTIONS) {
    evhtp_send_reply(req, EVHTP_RES_OK);
    return;
  }

  // create default result
  j_res = create_default_result(status_code);

  // create error
  if(err_code == 0) {
    code = status_code;
  }
  else {
    code = err_code;
  }
  j_tmp = json_pack("{s:i, s:s}",
      "code",     code,
      "message",  err_msg? : ""
      );
  json_object_set_new(j_res, "error", j_tmp);

  res = json_dumps(j_res, JSON_ENCODE_ANY);
  json_decref(j_res);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, status_code);
  sfree(res);

  return;
}

json_t* create_default_result(int code)
{
  json_t* j_res;
  char* timestamp;

  timestamp = get_utc_timestamp();

  j_res = json_pack("{s:s, s:s, s:i}",
      "api_ver",    API_VER,
      "timestamp",  timestamp,
      "statuscode", code
      );
  sfree(timestamp);

  return j_res;
}

json_t* get_json_from_request_data(evhtp_request_t* req)
{
  const char* tmp_const;
  char* tmp;
  json_t* j_data;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    return NULL;
  }

  // create json
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    return NULL;
  }

  return j_data;
}

/**
 * http request handler
 * ^/ping
 * @param req
 * @param data
 */
static void cb_htp_ping(evhtp_request_t *req, void *a)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_ping.");

  // create result
  j_tmp = json_pack("{s:s}",
      "message",  "pong"
      );

  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // send response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * http request handler
 * ^/sip/peers
 * @param req
 * @param data
 */
static void cb_htp_sip_peers(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_peers.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_sip_peers(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/peers/
 * @param req
 * @param data
 */
static void cb_htp_sip_peers_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_peers_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_sip_peers_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}


/**
 * http request handler
 * ^/databases
 * @param req
 * @param data
 */
static void cb_htp_databases(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_res;
  json_t* j_tmp;

  slog(LOG_WARNING, "Deprecated api.");
  simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_databases.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    j_tmp = get_databases_all_key();
    if(j_tmp == NULL) {
      simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", j_tmp);

    simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  return;
}

/**
 * http request handler
 * ^/databases/
 * @param req
 * @param data
 */
static void cb_htp_databases_key(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_data;
  json_t* j_res;
  json_t* j_tmp;
  const char* tmp_const;
  char* tmp;

  slog(LOG_WARNING, "Deprecated api.");
  simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_databases_key.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    // get data
    tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
    if(tmp_const == NULL) {
      simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // create json
    tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
    slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
    j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
    sfree(tmp);
    if(j_data == NULL) {
      simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // get value
    j_tmp = get_database_info(json_string_value(json_object_get(j_data, "key")));
    json_decref(j_data);
    if(j_tmp == NULL) {
      simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", j_tmp);

    simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  return;
}

/**
 * http request handler
 * ^/registries
 * @param req
 * @param data
 */
static void cb_htp_sip_registries(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_registries.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_sip_registries(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/registries/
 * @param req
 * @param data
 */
static void cb_htp_sip_registries_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_registries_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_sip_registries_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/queues$
 * @param req
 * @param data
 */
static void cb_htp_queue_queues(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_queues.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_queues(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_queue_queues(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/queues/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_queues_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_queues_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_queues_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_queue_queues_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_queue_queues_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/setting$
 * @param req
 * @param data
 */
static void cb_htp_queue_setting(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_setting.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_setting(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_queue_setting(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/settings/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_settings_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_settings_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_voicemail_settings_detail(req, data);
    return;
  }
  else if (method == htp_method_DELETE) {
    htp_delete_voicemail_settings_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/settings$
 * @param req
 * @param data
 */
static void cb_htp_queue_settings(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_settings.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_voicemail_settings(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/statuses$
 * @param req
 * @param data
 */
static void cb_htp_queue_statuses(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_statuses.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_statuses(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/statuses/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_statuses_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_statuses_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_statuses_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}


/**
 * http request handler
 * ^/queue_members
 * @param req
 * @param data
 */
static void cb_htp_queue_members(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_members.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_members(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/members/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_members_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_members_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_members_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/entries$
 * @param req
 * @param data
 */
static void cb_htp_queue_entries(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_entries.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_entries(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/entries/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_entries_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_entries_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_queue_entries_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/core/channels
 * @param req
 * @param data
 */
static void cb_htp_core_channels(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_channels.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_core_channels(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/channels/
 * @param req
 * @param data
 */
static void cb_htp_core_channels_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_channels_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_core_channels_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_core_channels_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/modules$
 * @param req
 * @param data
 */
static void cb_htp_core_modules(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_modules.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_core_modules(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/modules/(.*)
 * @param req
 * @param data
 */
static void cb_htp_core_modules_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_modules_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_core_modules_detail(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_core_modules_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_core_modules_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_core_modules_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}


/**
 * http request handler
 * ^/agents
 * @param req
 * @param data
 */
static void cb_htp_agent_agents(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_agents.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_agent_agents(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;

}

/**
 * http request handler
 * ^/agent/agents/(id)
 * @param req
 * @param data
 */
static void cb_htp_agent_agents_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_agents_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_agent_agents_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/core/systems
 * @param req
 * @param data
 */
static void cb_htp_core_systems(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_systems.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_core_systems(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;

}

/**
 * http request handler
 * ^/core/systems/
 * @param req
 * @param data
 */
static void cb_htp_core_systems_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_systems_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_core_systems_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/device_states
 * @param req
 * @param data
 */
static void cb_htp_device_states(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_device_states.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    j_tmp = get_device_states_all_device();
    if(j_tmp == NULL) {
      simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", json_object());
    json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

    simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/device_states/
 * @param req
 * @param data
 */
static void cb_htp_device_states_detail(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_data;
  json_t* j_res;
  json_t* j_tmp;
  const char* tmp_const;
  char* tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_systems_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    // get data
    tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
    if(tmp_const == NULL) {
      simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // create json
    tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
    slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
    j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
    sfree(tmp);
    if(j_data == NULL) {
      simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // get value
    j_tmp = get_device_state_info(
        json_string_value(json_object_get(j_data, "device"))
        );
    json_decref(j_data);
    if(j_tmp == NULL) {
      simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", j_tmp);

    simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  return;
}

/**
 * http request handler
 * ^/park/parking_lots$
 * @param req
 * @param data
 */
static void cb_htp_park_parkinglots(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parking_lots.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_park_parkinglots(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/park/parking_lots/(.*)
 * @param req
 * @param data
 */
static void cb_htp_park_parkinglots_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parking_lots_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_park_parkinglots_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/park/parkedcalls$
 * @param req
 * @param data
 */
static void cb_htp_park_parkedcalls(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parkedcalls.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_park_parkedcalls(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/park/parkedcalls/(.*)
 * @param req
 * @param data
 */
static void cb_htp_park_parkedcalls_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parkedcalls_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_park_parkedcalls_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/voicemail/users
 * @param req
 * @param data
 */
static void cb_htp_voicemail_users(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_users.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_voicemail_users(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    htp_post_voicemail_users(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/setting$
 * @param req
 * @param data
 */
static void cb_htp_voicemail_setting(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_setting.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_voicemail_setting(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_voicemail_setting(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/settings$
 * @param req
 * @param data
 */
static void cb_htp_voicemail_settings(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_settings.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_voicemail_settings(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/settings/(.*)
 * @param req
 * @param data
 */
static void cb_htp_voicemail_settings_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_settings_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_voicemail_settings_detail(req, data);
    return;
  }
  else if (method == htp_method_DELETE) {
    htp_delete_voicemail_settings_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/users/<detail>
 * @param req
 * @param data
 */
static void cb_htp_voicemail_users_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_users_context_mailbox.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_voicemail_users_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    htp_put_voicemail_users_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_voicemail_users_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}


/**
 * http request handler
 * ^/voicemail/vms
 * @param req
 * @param data
 */
static void cb_htp_voicemail_vms(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_vms.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_voicemail_vms(req, NULL);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/vms/<msgname>
 * @param req
 * @param data
 */
static void cb_htp_voicemail_vms_msgname(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_vms_msgname.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_voicemail_vms_msgname(req, NULL);
    return;
  }
  else if(method == htp_method_DELETE) {
    htp_delete_voicemail_vms_msgname(req, NULL);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/pjsip/endpoints
 * @param req
 * @param data
 */
static void cb_htp_pjsip_endpoints(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_endpoints.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    htp_get_pjsip_endpoints(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/endpoints/(*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_endpoints_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_endpoints_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_pjsip_endpoints_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/aors$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_aors(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_aors.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_pjsip_aors(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/aors/(.*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_aors_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_aors_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_pjsip_aors_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/auths$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_auths(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_auths.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_pjsip_auths(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/auths/(.*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_auths_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_auths_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_pjsip_auths_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/contacts$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_contacts(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_contacts.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_pjsip_contacts(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/contacts/(.*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_contacts_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_contacts_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    htp_get_pjsip_contacts_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}
