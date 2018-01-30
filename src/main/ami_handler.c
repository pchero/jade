
/*!
  \file   ami_handler.c
  \brief  

  \author Sungtae Kim
  \date   Dec 20, 2013

 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <termios.h>
#include <fcntl.h>
#include <stdbool.h>
#include <event2/event.h>
#include <jansson.h>
#include <errno.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "ami_handler.h"
#include "action_handler.h"
#include "ob_ami_handler.h"
#include "ob_dialing_handler.h"
#include "ami_event_handler.h"

#define DLE     0x0f  // Data link escape
#define DO      0xfd
#define WONT    0xfc
#define WILL    0xfb
#define DONT    0xfe
#define CMD     0xff

#define CMD_ECHO 1
#define CMD_WINDOW_SIZE 31

#define MAX_AMI_ITEM_LEN   40960

static int g_ami_socket = -1;


/**
 * AMI command msg send handler.
 * send_ami_cmd
 * @param data
 */
bool send_ami_cmd(json_t* j_cmd)
{
  int ret;
  json_t* j_val;
  const char* key;
  int type;
  char* tmp;
  char* cmd;
  char* cmd_sub;

  // just for log
  if(j_cmd == NULL) {
    return false;
  }

  // Get action
  j_val = json_object_get(j_cmd, "Action");
  if(j_val == NULL) {
    slog(LOG_ERR, " not get the action.");
    return false;
  }

  asprintf(&cmd, "Action: %s\r\n", json_string_value(j_val));
  slog(LOG_DEBUG, "AMI Action command. action[%s]", json_string_value(j_val));

  json_object_foreach(j_cmd, key, j_val){
    ret = strcmp(key, "Action");
    if(ret == 0) {
      continue;
    }

    ret = strcmp(key, "Variables");
    if(ret == 0) {
      cmd_sub = get_variables_ami_str_from_object(j_val);
      asprintf(&tmp, "%s%s", cmd, cmd_sub);
      sfree(cmd);
      sfree(cmd_sub);
      cmd = tmp;
      continue;
    }

    type = json_typeof(j_val);
    switch(type) {
      case JSON_REAL:
      case JSON_INTEGER:
      {
        asprintf(&cmd_sub, "%s: %lld", key, json_integer_value(j_val));
      }
      break;

      case JSON_FALSE:
      case JSON_TRUE:
      case JSON_STRING:
      {
        asprintf(&cmd_sub, "%s: %s", key, json_string_value(j_val));
      }
      break;

      default:
      {
        slog(LOG_WARNING, "Invalid type. Set to <unknown>. type[%d]", type);
        asprintf(&cmd_sub, "%s: %s", key, "<unknown>");
      }
      break;
    }

    asprintf(&tmp, "%s%s\r\n", cmd, cmd_sub);
    sfree(cmd_sub);
    sfree(cmd);
    cmd = tmp;
  }

  asprintf(&tmp, "%s\r\n", cmd);
  sfree(cmd);

  cmd = tmp;
  ret = send_ami_cmd_raw(cmd);
  sfree(cmd);
  if(ret < 0) {
    return false;
  }

  return true;
}

/**
 * Send raw format ami message.
 * @param cmd
 * @return Success: Size of sent message.\n
 * Fail: -1
 */
int send_ami_cmd_raw(const char* cmd)
{
  int ret;
  
  if(cmd == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return -1;
  }
  slog(LOG_DEBUG, "Fired send_ami_cmd_raw. cmd[%s]", cmd);
  
  ret = send(g_ami_socket, cmd, strlen(cmd), 0);
  if(ret < 0) {
    return -1;
  }
  
  return ret;
}

/**
 *
 * @param msg
 * @return
 */
json_t* parse_ami_msg(const char* msg)
{
	json_t* j_tmp;
	char tmp[MAX_AMI_ITEM_LEN];
	int ret;
	int i;
	int j;
  char* key;
  char* value;
  char* dump;
  int parsed;

	if(msg == NULL) {
		slog(LOG_WARNING, "Wrong input parameter.");
		return NULL;
	}

  j_tmp = json_object();
  bzero(tmp, sizeof(tmp));
  for(i = 0, j = 0; i < strlen(msg); i++) {
    parsed = false;

    if((msg[i] != '\r') || (msg[i+1] != '\n')) {
      tmp[j] = msg[i];
      j++;
      continue;
    }

		// check /r/n/r/n
		ret = strlen(tmp);
		if(ret == 0) {
			break;
		}

		// get key/value
		value = strdup(tmp);
		dump = value;
		key = strsep(&value, ":");
		if(key == NULL) {
			sfree(dump);
			continue;
		}
		trim(key);
		trim(value);

		// check Variable
		ret = strcasecmp(key, "Variable");
		if(ret == 0) {
			if(json_object_get(j_tmp, "Variable") == NULL) {
				json_object_set_new(j_tmp, "Variable", json_array());
			}
			json_array_append_new(json_object_get(j_tmp, "Variable"), json_string(value));
			parsed = true;
		}

		// check Output
		ret = strcasecmp(key, "Output");
		if(ret == 0) {
		  if(json_object_get(j_tmp, "Output") == NULL) {
		    json_object_set_new(j_tmp, "Output", json_array());
		  }
		  json_array_append_new(json_object_get(j_tmp, "Output"), json_string(value));
		  parsed = true;
		}

		if(parsed != true) {
			json_object_set_new(j_tmp, key, json_string(value));
		}


		sfree(dump);
		memset(tmp, 0x00, sizeof(tmp));
		j = 0;
		i++;
		continue;
	}

  return j_tmp;
}

json_t* parse_ami_agi_env(const char* msg)
{
  char tmp[40960];
  int i;
  int j;
  char* value;
  char* dump;
  char* key;
  json_t* j_res;

  if(msg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.\n");
    return NULL;
  }

  j = 0;
  bzero(tmp, sizeof(tmp));
  j_res = json_object();
  for(i = 0; i < strlen(msg); i++) {
    tmp[j] = msg[i];
    j++;
    if(msg[i] != '\n') {
      continue;
    }

    // get key/value
    value = strdup(tmp);
    dump = value;
    key = strsep(&value, ":");
    bzero(tmp, sizeof(tmp));
    j = 0;

    if((key == NULL) || (value == NULL)) {
      sfree(dump);
      continue;
    }
    trim(key);
    trim(value);

    json_object_set_new(j_res, key, json_string(value));
    free(dump);
  }

  return j_res;

}


/**
 * Set ami socket.
 * @param sock
 */
void set_ami_socket(int sock)
{
  g_ami_socket = sock;
}


