
#ifndef __AMI_HANDLER_H__
#define __AMI_HANDLER_H__

#include <stdbool.h>
#include <jansson.h>

bool init_ami_handler(void);
int send_ami_cmd_raw(const char* cmd);
bool send_ami_cmd(json_t* j_cmd);

#endif
