/*
 * ami_action_handler.c
 *
 *  Created on: Dec 15, 2017
 *      Author: pchero
 */

#include <stdio.h>
#include <jansson.h>

#include "slog.h"
#include "ami_handler.h"
#include "resource_handler.h"

#include "ami_action_handler.h"


int ami_action_hangup(const char* channel)
{
  json_t* j_tmp;
  int ret;

  if(channel == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_hangup. channel[%s]", channel);

  // create hangup request
  j_tmp = json_pack("{s:s, s:s}",
      "Action",   "Hangup",
      "Channel",  channel
      );

  // send hangup request
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action");
    return false;
  }

  return true;
}

/**
 * AMI event handler.
 * Action: hangup
 * @param j_msg
 */
int ami_action_hangup_by_uniqueid(const char* unique_id)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_action_hangup_by_uniqueid. unique_id[%s]", unique_id);

  j_tmp = get_channel_info(unique_id);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get channel info.");
    return false;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "channel"));
  ret = ami_action_hangup(tmp_const);
  json_decref(j_tmp);
  if(ret == false) {
    return false;
  }

  return true;
}
