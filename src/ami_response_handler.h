/*
 * ami_response_handler.h
 *
 *  Created on: Mar 31, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_AMI_RESPONSE_HANDLER_H_
#define BACKEND_SRC_AMI_RESPONSE_HANDLER_H_

#include <jansson.h>

ACTION_RES ami_response_handler_corestatus(json_t* j_action, json_t* j_msg);
ACTION_RES ami_response_handler_coresettings(json_t* j_action, json_t* j_msg);


#endif /* BACKEND_SRC_AMI_RESPONSE_HANDLER_H_ */
