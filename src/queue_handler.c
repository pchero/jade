/*
 * queue_handler.c
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */


#include "slog.h"
#include "http_handler.h"
#include "utils.h"

#include "queue_handler.h"
#include "resource_handler.h"



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
