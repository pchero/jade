/*
 * me_handler.h
 *
 *  Created on: Feb 4, 2018
 *      Author: pchero
 */

#ifndef SRC_MAIN_ME_HANDLER_H_
#define SRC_MAIN_ME_HANDLER_H_

#include <stdbool.h>

bool me_init_handler(void);
bool me_reload_handler(void);
void me_term_handler(void);


// http handlers
void me_htp_get_me_info(evhtp_request_t *req, void *data);

void me_htp_get_me_buddies(evhtp_request_t *req, void *data);
void me_htp_post_me_buddies(evhtp_request_t *req, void *data);

void me_htp_get_me_buddies_detail(evhtp_request_t *req, void *data);
void me_htp_put_me_buddies_detail(evhtp_request_t *req, void *data);
void me_htp_delete_me_buddies_detail(evhtp_request_t *req, void *data);

void me_htp_get_me_calls(evhtp_request_t *req, void *data);
void me_htp_post_me_calls(evhtp_request_t *req, void *data);

void me_htp_get_me_chats(evhtp_request_t *req, void *data);
void me_htp_post_me_chats(evhtp_request_t *req, void *data);

void me_htp_get_me_chats_detail(evhtp_request_t *req, void *data);
void me_htp_put_me_chats_detail(evhtp_request_t *req, void *data);
void me_htp_delete_me_chats_detail(evhtp_request_t *req, void *data);

void me_htp_post_me_chats_detail_messages(evhtp_request_t *req, void *data);
void me_htp_get_me_chats_detail_messages(evhtp_request_t *req, void *data);

#endif /* SRC_MAIN_ME_HANDLER_H_ */
