
#ifndef __AMI_HANDLER_H__
#define __AMI_HANDLER_H__

#include <stdbool.h>
#include <jansson.h>

json_t* parse_ami_msg(const char* msg);
int send_ami_cmd_raw(const char* cmd);
bool send_ami_cmd(json_t* j_cmd);

void set_ami_socket(int socket);

#endif
