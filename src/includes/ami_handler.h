
#ifndef __AMI_HANDLER_H__
#define __AMI_HANDLER_H__

#include <stdbool.h>
#include <jansson.h>

json_t* ami_parse_msg(const char* msg);
json_t* ami_parse_agi_env(const char* msg);

int ami_send_cmd_raw(const char* cmd);
bool ami_send_cmd(json_t* j_cmd);

void ami_set_socket(int socket);

#endif
