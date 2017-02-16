/*
 * ob_ami_handler.h
 *
 *  Created on: Feb 15, 2017
 *      Author: pchero
 */

#ifndef BACKEND_SRC_OB_AMI_HANDLER_H_
#define BACKEND_SRC_OB_AMI_HANDLER_H_

void ob_ami_response_handler_originate(json_t* j_msg);


bool originate_to_exten(json_t* j_dialing);
bool originate_to_application(json_t* j_dialing);


#endif /* BACKEND_SRC_OB_AMI_HANDLER_H_ */
