/*
 * user_handler.h
 *
 *  Created on: Feb 1, 2018
 *      Author: pchero
 */

#ifndef SRC_MAIN_USER_HANDLER_H_
#define SRC_MAIN_USER_HANDLER_H_

#include <stdbool.h>

bool init_user_handler(void);
void term_user_handler(void);
bool reload_user_handler(void);


void htp_post_user_login(evhtp_request_t *req, void *data);
void htp_delete_user_login(evhtp_request_t *req, void *data);

void htp_get_user_contacts(evhtp_request_t *req, void *data);
void htp_post_user_contacts(evhtp_request_t *req, void *data);

void htp_get_user_contacts_detail(evhtp_request_t *req, void *data);
void htp_put_user_contacts_detail(evhtp_request_t *req, void *data);
void htp_delete_user_contacts_detail(evhtp_request_t *req, void *data);

#endif /* SRC_MAIN_USER_HANDLER_H_ */
