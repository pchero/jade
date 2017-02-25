/*
 * dialing_handler.h
 *
 *  Created on: Nov 21, 2015
 *    Author: pchero
 */

#ifndef SRC_DIALING_HANDLER_H_
#define SRC_DIALING_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

typedef enum _E_DIALING_STATUS_T
{
  E_DIALING_NONE          = 0,  ///< None state
  E_DIALING_ORIGINATE_REQUEST   = 1,
  E_DIALING_DIAL_BEGIN,
  E_DIALING_CHANNEL_CREATE,
  E_DIALING_DIAL_END,
  E_DIALING_ORIGINATE_RESPONSE  = 5,
  E_DIALING_HANGUP,

  E_DIALING_ERROR                       = 10,             ///< error
  E_DIALING_ORIGINATE_REQUEST_FAILED    = 11,   ///< Could not send originate request
  E_DIALING_ORIGINATE_RESPONSE_FAILED   = 12,    ///< originate response was wrong
} E_DIALING_STATUS_T;

typedef struct _rb_dialing{
  char* uuid;         ///< dialing uuid(channel's unique id)
  char* name;         ///< dialing name(channel's name)
  E_DIALING_STATUS_T status;  ///< dialing status

  char* tm_create;
  char* tm_update;
  char* tm_delete;
  struct timespec timeptr_update; ///< timestamp for timeout

  json_t* j_dialing;  ///< dialing info(result).
  json_t* j_event;  ///< current channel status info(the latest event)
  json_t* j_events;    ///< event info(json array).
} rb_dialing;

json_t* create_ob_dialing(const char* dialing_uuid, json_t* j_camp, json_t* j_plan, json_t* j_dlma, json_t* j_dest, json_t* j_dl_list, json_t* j_dial);
bool delete_ob_dialing(const char* uuid);
bool is_exist_ob_dialing(const char* uuid);
bool update_ob_dialing_hangup(const char* uuid, int hangup, const char* hangup_detail);


void rb_dialing_destory(rb_dialing* dialing);

rb_dialing* rb_dialing_find_chan_name(const char* chan);
rb_dialing* rb_dialing_find_chan_uuid(const char* chan);

struct ao2_iterator rb_dialing_iter_init(void);
rb_dialing* rb_dialing_iter_next(struct ao2_iterator *iter);

//json_t* rb_dialing_get_all_for_cli(void);
json_t* rb_dialing_get_info_for_cli(const char* uuid);

bool rb_dialing_update_name(rb_dialing* dialing, const char* name);
bool update_ob_dialing_status(const char* uuid, E_DIALING_STATUS_T status);
bool rb_dialing_update_events_append(rb_dialing* dialing, json_t* j_evt);
bool rb_dialing_update_dialing_update(rb_dialing* dialing, json_t* j_dialing);
bool rb_dialing_update_current_update(rb_dialing* dialing, json_t* j_evt);
bool rb_dialing_update_event_substitute(rb_dialing* dialing, json_t* j_evt);

int get_ob_dialing_count_by_camp_uuid(const char* camp_uuid);
json_t* get_ob_dialing_by_action_id(const char* action_id);


#endif /* SRC_DIALING_HANDLER_H_ */
