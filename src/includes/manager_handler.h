/*
 * manager_handler.h
 *
 *  Created on: Apr 20, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_MANAGER_HANDLER_H_
#define SRC_INCLUDES_MANAGER_HANDLER_H_

#include <stdbool.h>

bool manager_init_handler(void);
bool manager_term_handler(void);
bool manager_reload_handler(void);

void manager_htp_post_manager_login(evhtp_request_t *req, void *data);


#endif /* SRC_INCLUDES_MANAGER_HANDLER_H_ */
