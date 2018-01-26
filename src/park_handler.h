/*
 * park_handler.h
 *
 *  Created on: Dec 14, 2017
 *      Author: pchero
 */

#ifndef SRC_PARK_HANDLER_H_
#define SRC_PARK_HANDLER_H_

#include <evhtp.h>

void htp_get_park_parkinglots(evhtp_request_t *req, void *data);
void htp_post_park_parkinglots(evhtp_request_t *req, void *data);

void htp_get_park_parkinglots_detail(evhtp_request_t *req, void *data);
void htp_put_park_parkinglots_detail(evhtp_request_t *req, void *data);
void htp_delete_park_parkinglots_detail(evhtp_request_t *req, void *data);

void htp_get_park_parkedcalls(evhtp_request_t *req, void *data);

void htp_get_park_parkedcalls_detail(evhtp_request_t *req, void *data);

void htp_get_park_config(evhtp_request_t *req, void *data);
void htp_put_park_config(evhtp_request_t *req, void *data);

void htp_get_park_configs(evhtp_request_t *req, void *data);

void htp_get_park_configs_detail(evhtp_request_t *req, void *data);
void htp_delete_park_configs_detail(evhtp_request_t *req, void *data);

void htp_get_park_settings(evhtp_request_t *req, void *data);
void htp_post_park_settings(evhtp_request_t *req, void *data);

void htp_get_park_settings_detail(evhtp_request_t *req, void *data);
void htp_put_park_settings_detail(evhtp_request_t *req, void *data);
void htp_delete_park_settings_detail(evhtp_request_t *req, void *data);


#endif /* SRC_PARK_HANDLER_H_ */
