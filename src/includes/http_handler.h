/*
 * http_handler.h
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_HTTP_HANDLER_H_
#define BACKEND_SRC_HTTP_HANDLER_H_

#include <evhtp.h>
#include <stdbool.h>
#include <jansson.h>

#define DEF_REG_UUID "[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}"

bool init_http_handler(void);
void term_http_handler(void);

json_t* http_create_default_result(int code);

void http_simple_response_error(evhtp_request_t *req, int status_code, int err_code, const char* err_msg);
void http_simple_response_normal(evhtp_request_t *req, json_t* j_msg);

json_t* http_get_json_from_request_data(evhtp_request_t* req);
char* http_get_text_from_request_data(evhtp_request_t* req);
char* http_get_parsed_detail(evhtp_request_t* req);

bool http_get_htp_id_pass(evhtp_request_t* req, char** agent_uuid, char** agent_pass);

#endif /* BACKEND_SRC_HTTP_HANDLER_H_ */
