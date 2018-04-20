/*
 * admin_handler.h
 *
 *  Created on: Apr 19, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_ADMIN_HANDLER_H_
#define SRC_INCLUDES_ADMIN_HANDLER_H_

#include <stdbool.h>

bool admin_init_handler(void);
bool admin_term_handler(void);
bool admin_reload_handler(void);


// http handlers
void admin_htp_post_admin_login(evhtp_request_t *req, void *data);

void amd_htp_get_admin_users(evhtp_request_t *req, void *data);

#endif /* SRC_INCLUDES_ADMIN_HANDLER_H_ */
