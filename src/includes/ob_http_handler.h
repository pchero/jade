/*
 * ob_http_handler.h
 *
 *  Created on: Mar 15, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_OB_HTTP_HANDLER_H_
#define BACKEND_SRC_OB_HTTP_HANDLER_H_

#include <evhtp.h>

// campaigns
void ob_cb_htp_ob_campaigns(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_campaigns_all(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_campaigns_detail(evhtp_request_t *req, void *data);

// plans
void ob_cb_htp_ob_plans(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_plans_all(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_plans_detail(evhtp_request_t *req, void *data);

// destinations
void ob_cb_htp_ob_destinations(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_destinations_all(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_destinations_detail(evhtp_request_t *req, void *data);

// dlmas
void ob_cb_htp_ob_dlmas(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_dlmas_all(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_dlmas_detail(evhtp_request_t *req, void *data);

// dls
void ob_cb_htp_ob_dls(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_dls_all(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_dls_detail(evhtp_request_t *req, void *data);

// dialings
void ob_cb_htp_ob_dialings(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_dialings_all(evhtp_request_t *req, void *data);
void ob_cb_htp_ob_dialings_detail(evhtp_request_t *req, void *data);


#endif /* BACKEND_SRC_OB_HTTP_HANDLER_H_ */
