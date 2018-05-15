/*
 * call_handler.c
 *
 * Asterisk's channel resource handler.
 * And provide useful interface for call/channel.
 *  Created on: Apr 13, 2018
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>


#include "slog.h"
#include "resource_handler.h"
#include "publication_handler.h"
#include "utils.h"
#include "dialplan_handler.h"
#include "ami_action_handler.h"
#include "core_handler.h"

#include "call_handler.h"


bool call_hangup_by_unique_id(const char* unique_id)
{
  int ret;
  const char* channel;
  json_t* j_tmp;

  if(unique_id == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = core_get_channel_info(unique_id);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Could not get channel info.");
    return false;
  }

  channel = json_string_value(json_object_get(j_tmp, "channel"));
  if(channel == NULL) {
    slog(LOG_NOTICE, "Could not get channel name.");
    json_decref(j_tmp);
    return false;
  }

  ret = ami_action_hangup(channel);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not send hangup request.");
    return false;
  }

  return true;
}


/**
 * Originate call from the source device to destination device.
 * @param source
 * @param destination
 * @return
 */
bool call_originate_call_to_device(const char* source, const char* destination)
{
  int ret;
  char* data;
  char* channel;
  char* dpma_uuid;

  if((source == NULL) || (destination == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired call_originate_call_to_device. source[%s], destination[%s]", source, destination);

  // get default dpma for originate to device
  dpma_uuid = dialplan_get_default_dpma_originate_to_device();
  if(dpma_uuid == NULL) {
    slog(LOG_ERR, "Could not get default dpma originate to device.");
    return false;
  }

  // create data
  asprintf(&data, "agi:async,%s,%s,%s",
      DEF_DIALPLAN_JADE_AGI_NAME,
      dpma_uuid,
      destination
      );
  sfree(dpma_uuid);

  // create channel
  asprintf(&channel, "PJSIP/%s", source);

  // send request
  ret = ami_action_originate_application(channel, "AGI", data);
  sfree(channel);
  sfree(data);
  if(ret == false) {
    slog(LOG_ERR, "Could not send originate request to device info.");
    return false;
  }

  return true;
}












