/*
 * queue_handler.c
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */


#include "slog.h"
#include "http_handler.h"
#include "utils.h"
#include "config.h"

#include "queue_handler.h"
#include "resource_handler.h"

#define DEF_QUEUE_CONFNAME  "queues.conf"

static bool create_queue_info(json_t* j_data);
static json_t* create_queue_info_json(json_t* j_data);



/**
 * htp request handler.
 * request: GET ^/queue/entries$
 * @param req
 * @param data
 */
void htp_get_queue_entries(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_entries.");

  // get info
  j_tmp = get_queue_entries_all();

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/queue/entries/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_queue_entries_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_entries_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_parse(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_entry_info(name);
  sfree(name);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

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
 * request: GET ^/queue/members$
 * @param req
 * @param data
 */
void htp_get_queue_members(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_members.");

  // get info
  j_tmp = get_queue_members_all();

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/queue/members/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_queue_members_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* name;
  char* queue;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_members_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_parse(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // queue_name parse
  tmp_const = evhtp_kv_find(req->uri->query, "queue_name");
  queue = uri_parse(tmp_const);
  if(queue == NULL) {
    slog(LOG_ERR, "Could not get queue info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_member_info(name, queue);
  sfree(name);
  sfree(queue);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

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
 * request: GET ^/queue/queues$
 * @param req
 * @param data
 */
void htp_get_queue_queues(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_queues.");

  // get info
  j_tmp = get_queue_params_all();

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * htp request handler.
 * request: POST ^/queue/queues$
 * @param req
 * @param data
 */
void htp_post_queue_queues(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_queue_queues.");

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    slog(LOG_ERR, "Could not get correct data from request.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create queue
  ret = create_queue_info(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not get create queue info.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/queue/queues/(.*) request handler.
 * @param req
 * @param data
 */
void htp_get_queue_queues_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* tmp_const;
  char* name;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_queue_queues_detail.");

  // name parse
  tmp_const = req->uri->path->file;
  name = uri_parse(tmp_const);
  if(name == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_queue_param_info(name);
  sfree(name);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get name info.");
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * Create queue info.
 * @param j_data
 * @return
 */
static bool create_queue_info(json_t* j_data)
{
  const char* name;
  json_t* j_tmp;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get queue name
  name = json_string_value(json_object_get(j_data, "name"));
  if(name == NULL) {
    slog(LOG_ERR, "Could not get queue name.");
    return false;
  }

  // create json
  j_tmp = create_queue_info_json(j_data);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not create queue info json.");
    return false;
  }

  // create queue info
  ret = create_ast_current_config_section_data(DEF_QUEUE_CONFNAME, name, j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not create queue info.");
    return false;
  }

  return true;
}

/**
 * Create json info for queue creation.
 * @param j_data
 * @return
 */
static json_t* create_queue_info_json(json_t* j_data)
{
  json_t* j_res;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // currently, just copy the data.
  // may need to validate all the options later.
  j_res = json_deep_copy(j_data);

  // remove the name
  json_object_del(j_res, "name");

  return j_res;
}

