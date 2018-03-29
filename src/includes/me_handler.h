/*
 * me_handler.h
 *
 *  Created on: Feb 4, 2018
 *      Author: pchero
 */

#ifndef SRC_MAIN_ME_HANDLER_H_
#define SRC_MAIN_ME_HANDLER_H_

#include <stdbool.h>

bool init_me_handler(void);
bool reload_me_handler(void);
void term_me_handler(void);


void htp_get_me_info(evhtp_request_t *req, void *data);

void htp_get_me_chats(evhtp_request_t *req, void *data);
void htp_post_me_chats(evhtp_request_t *req, void *data);


#endif /* SRC_MAIN_ME_HANDLER_H_ */
