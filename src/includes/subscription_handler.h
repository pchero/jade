/*
 * subscription_handler.h
 *
 *  Created on: Apr 15, 2018
 *      Author: pchero
 */

#ifndef SRC_INCLUDES_SUBSCRIPTION_HANDLER_H_
#define SRC_INCLUDES_SUBSCRIPTION_HANDLER_H_


#include <stdbool.h>

bool subscription_init_handler(void);
bool subscription_term_handler(void);
bool subscription_reload_handler(void);


#endif /* SRC_INCLUDES_SUBSCRIPTION_HANDLER_H_ */
