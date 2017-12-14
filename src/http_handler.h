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

json_t* create_default_result(int code);

void simple_response_error(evhtp_request_t *req, int status_code, int err_code, const char* err_msg);
void simple_response_normal(evhtp_request_t *req, json_t* j_msg);

json_t* get_json_from_request_data(evhtp_request_t* req);

#endif /* BACKEND_SRC_HTTP_HANDLER_H_ */
