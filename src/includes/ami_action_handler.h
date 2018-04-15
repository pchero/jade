/*
 * ami_action_handler.h
 *
 *  Created on: Dec 15, 2017
 *      Author: pchero
 */

#ifndef SRC_AMI_ACTION_HANDLER_H_
#define SRC_AMI_ACTION_HANDLER_H_

#include <stdbool.h>

bool ami_action_agi(const char* channel, const char* cmd, const char* cmd_id);
bool ami_action_hangup(const char* channel);
bool ami_action_hangup_by_uniqueid(const char* unique_id);
bool ami_action_modulecheck(const char* name);
bool ami_action_moduleload(const char* name, const char* type);
bool ami_action_originate_application(const char* channel, const char* application, const char* data);
bool ami_action_setvar(const char* channel, const char* key, const char* value);


#endif /* SRC_AMI_ACTION_HANDLER_H_ */
