/*
 * ami_action_handler.h
 *
 *  Created on: Dec 15, 2017
 *      Author: pchero
 */

#ifndef SRC_AMI_ACTION_HANDLER_H_
#define SRC_AMI_ACTION_HANDLER_H_


int ami_action_hangup(const char* channel);
int ami_action_hangup_by_uniqueid(const char* unique_id);
int ami_action_moduleload(const char* name, const char* type);



#endif /* SRC_AMI_ACTION_HANDLER_H_ */
