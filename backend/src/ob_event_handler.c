/*
 * event_handler.c
 *
 *  Created on: Nov 8, 2015
 *    Author: pchero
 */


#include <stdbool.h>
#include <jansson.h>
#include <event2/event.h>
#include <event2/thread.h>
#include <errno.h>
#include <string.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "db_handler.h"
#include "ami_handler.h"
#include "ast_causes.h"
#include "ob_db_sql_create.h"
#include "ob_event_handler.h"
#include "ob_ami_handler.h"
#include "ob_dialing_handler.h"
#include "ob_campaign_handler.h"
#include "ob_dl_handler.h"
#include "ob_plan_handler.h"
#include "ob_destination_handler.h"

#define TEMP_FILENAME "/tmp/asterisk_outbound_tmp.txt"
#define DEF_EVENT_TIME_FAST "100000"
#define DEF_EVENT_TIME_SLOW "3000000"
#define DEF_ONE_SEC_IN_MICRO_SEC  1000000


extern struct event_base*  g_base;
extern app* g_app;

static bool init_outbound(void);
static bool init_outbound_db(void);

static void cb_campaign_start(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_campaign_starting(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_campaign_stopping(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_campaign_stopping_force(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_check_dialing_end(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_check_dialing_error(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_check_campaign_end(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_check_campaign_schedule_start(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_check_campaign_schedule_end(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
//static void cb_campaign_schedule_stopping(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);

//static json_t* get_queue_info(const char* uuid);

static void dial_desktop(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma);
static void dial_power(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma);
static void dial_predictive(json_t* j_camp, json_t* j_plan, json_t* j_dlma, json_t* j_dest);
static void dial_robo(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma);
static void dial_redirect(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma);

//json_t* get_queue_summary(const char* name);
//json_t* get_queue_param(const char* name);

//static bool write_result_json(json_t* j_res);

// todo
static int check_dial_avaiable_predictive(json_t* j_camp, json_t* j_plan, json_t* j_dlma, json_t* j_dest);

int run_outbound(void)
{
  int ret;
  int event_delay;
  const char* tmp_const;
  struct event* ev;
  struct timeval tm_fast;
  struct timeval tm_slow;

  // event delay fast.
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "event_time_fast"));
  if(tmp_const == NULL) {
    tmp_const = DEF_EVENT_TIME_FAST;
    slog(LOG_NOTICE, "Could not get correct event_time_fast value. Set default. event_time_fast[%s]", tmp_const);
  }
  slog(LOG_NOTICE, "Event delay time for fast event. event_time_fast[%s]", tmp_const);

  event_delay = atoi(tmp_const);
  tm_fast.tv_sec = event_delay / DEF_ONE_SEC_IN_MICRO_SEC;
  tm_fast.tv_usec = event_delay % DEF_ONE_SEC_IN_MICRO_SEC;

  // event delay slow.
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "event_time_slow"));
  if(tmp_const == NULL) {
    tmp_const = DEF_EVENT_TIME_SLOW;
    slog(LOG_NOTICE, "Could not get correct event_time_fast value. Set default. event_time_fast[%s]", tmp_const);
  }
  slog(LOG_NOTICE, "Event delay time for fast event. event_time_fast[%s]", tmp_const);

  event_delay = atoi(tmp_const);
  tm_slow.tv_sec = event_delay / DEF_ONE_SEC_IN_MICRO_SEC;
  tm_slow.tv_usec = event_delay % DEF_ONE_SEC_IN_MICRO_SEC;

  // init libevent
  ret = init_outbound();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound.");
    return false;
  }

  // check start.
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_campaign_start, NULL);
  event_add(ev, &tm_fast);

  // check starting
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_campaign_starting, NULL);
  event_add(ev, &tm_slow);

  // check stopping.
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_campaign_stopping, NULL);
  event_add(ev, &tm_slow);

  // check force stopping
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_campaign_stopping_force, NULL);
  event_add(ev, &tm_slow);

  // chceck dialing end
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_check_dialing_end, NULL);
  event_add(ev, &tm_fast);

  // check diaing error
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_check_dialing_error, NULL);
  event_add(ev, &tm_fast);

  // check end
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_check_campaign_end, NULL);
  event_add(ev, &tm_slow);

  // check campaign scheduling start
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_check_campaign_schedule_start, NULL);
  event_add(ev, &tm_slow);

  // check campaign scheduling end
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_check_campaign_schedule_end, NULL);
  event_add(ev, &tm_slow);

//  // check campaign scheduling for stop
//  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_campaign_schedule_stopping, NULL);
//  event_add(ev, &tm_slow);

//  event_base_loop(g_base, 0);

  return true;
}

static bool init_outbound_db(void)
{
  int ret;

  ret = db_exec(g_sql_ob_destination);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound database. table[destination]");
    return false;
  }

  ret = db_exec(g_sql_ob_dial_list);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound database. table[dial_list]");
    return false;
  }

  ret = db_exec(g_sql_ob_dl_list_ma);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound database. table[dl_list_ma]");
    return false;
  }

  ret = db_exec(g_sql_ob_plan);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound database. table[plan]");
    return false;
  }

  ret = db_exec(g_sql_ob_dialing);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound database. table[dialing]");
    return false;
  }

  ret = db_exec(g_sql_ob_dl_result);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound database. table[dl_result]");
    return false;
  }

  ret = db_exec(g_sql_ob_campaign);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound database. table[campaign]");
    return false;
  }

  return true;
}

static bool init_outbound(void)
{
  int ret;

  if(g_base == NULL) {
    slog(LOG_ERR, "Could not initiate libevent. err[%d:%s]", errno, strerror(errno));
    return false;
  }

  ret = init_outbound_db();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate outbound.");
    return false;
  }

  slog(LOG_NOTICE, "Initiated outbound.");

  return true;
}

void stop_outbound(void)
{
  struct timeval sec;

  sec.tv_sec = 0;
  sec.tv_usec = 0;

  event_base_loopexit(g_base, &sec);

  return;
}

/**
 *  @brief  Check start status campaign and trying to make a call.
 */
static void cb_campaign_start(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_camp;
  json_t* j_plan;
  json_t* j_dlma;
  json_t* j_dest;
  int dial_mode;

  j_camp = get_ob_campaign_for_dialing();
  if(j_camp == NULL) {
    // Nothing.
    return;
  }
//  slog(LOG_DEBUG, "Get campaign info. camp_uuid[%s], camp_name[%s]",
//      json_string_value(json_object_get(j_camp, "uuid")),
//      json_string_value(json_object_get(j_camp, "name"))
//      );

  // get plan
  j_plan = get_ob_plan(json_string_value(json_object_get(j_camp, "plan")));
  if(j_plan == NULL) {
    slog(LOG_WARNING, "Could not get plan info. Stopping campaign. camp_uuid[%s], plan_uuid[%s]",
        json_string_value(json_object_get(j_camp, "uuid")),
        json_string_value(json_object_get(j_camp, "plan"))
        );
    update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    json_decref(j_camp);
    return;
  }

  // get destination
  j_dest = get_ob_destination(json_string_value(json_object_get(j_camp, "dest")));
  if(j_dest == NULL) {
    slog(LOG_WARNING, "Could not get dest info. Stopping campaign. camp_uuid[%s], dest_uuid[%s]",
            json_string_value(json_object_get(j_camp, "uuid"))? : "",
            json_string_value(json_object_get(j_camp, "dest"))? : ""
            );
    update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    json_decref(j_camp);
    json_decref(j_plan);
    return;
  }

  // get dl_master_info
  j_dlma = get_ob_dlma(json_string_value(json_object_get(j_camp, "dlma")));
  if(j_dlma == NULL)
  {
    slog(LOG_ERR, "Could not find dial list master info. Stopping campaign. camp_uuid[%s], dlma_uuid[%s]",
        json_string_value(json_object_get(j_camp, "uuid")),
        json_string_value(json_object_get(j_camp, "dlma"))
        );
    update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    json_decref(j_camp);
    json_decref(j_plan);
    json_decref(j_dest);
    return;
  }
  slog(LOG_DEBUG, "Get dlma info. dlma_uuid[%s], dlma_name[%s]",
      json_string_value(json_object_get(j_dlma, "uuid")),
      json_string_value(json_object_get(j_dlma, "name"))
      );

  // get dial_mode
  dial_mode = json_integer_value(json_object_get(j_plan, "dial_mode"));
  if(dial_mode == E_DIAL_MODE_NONE) {
    slog(LOG_ERR, "Plan has no dial_mode. Stopping campaign. camp[%s], plan[%s]",
        json_string_value(json_object_get(j_camp, "uuid")),
        json_string_value(json_object_get(j_camp, "plan"))
        );

    update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    json_decref(j_camp);
    json_decref(j_plan);
    json_decref(j_dlma);
    json_decref(j_dest);
    return;
  }

  switch(dial_mode) {
    case E_DIAL_MODE_PREDICTIVE: {
      dial_predictive(j_camp, j_plan, j_dlma, j_dest);
    }
    break;

    case E_DIAL_MODE_DESKTOP: {
      dial_desktop(j_camp, j_plan, j_dlma);
    }
    break;

    case E_DIAL_MODE_POWER: {
      dial_power(j_camp, j_plan, j_dlma);
    }
    break;

    case E_DIAL_MODE_ROBO: {
      dial_robo(j_camp, j_plan, j_dlma);
    }
    break;

    case E_DIAL_MODE_REDIRECT: {
      dial_redirect(j_camp, j_plan, j_dlma);
    }
    break;

    default: {
      slog(LOG_ERR, "No match dial_mode. dial_mode[%d]", dial_mode);
    }
    break;
  }

  // release
  json_decref(j_camp);
  json_decref(j_plan);
  json_decref(j_dlma);
  json_decref(j_dest);

  return;
}

/**
 *  @brief  Check starting status campaign and update status to stopping or start.
 */
static void cb_campaign_starting(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_camps;
  json_t* j_camp;
  int size;
  int i;
  int ret;

  j_camps = get_ob_campaigns_by_status(E_CAMP_STARTING);
  if(j_camps == NULL) {
    // Nothing.
    return;
  }

  size = json_array_size(j_camps);
  for(i = 0; i < size; i++) {
    j_camp = json_array_get(j_camps, i);

    // check startable
    ret = is_startable_campgain(j_camp);
    if(ret == false) {
      continue;
    }

    // update campaign status
    slog(LOG_NOTICE, "update ob_campaign status to start. camp_uuid[%s], camp_name[%s]",
        json_string_value(json_object_get(j_camp, "uuid")),
        json_string_value(json_object_get(j_camp, "name"))
        );
    ret = update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_START);
    if(ret == false) {
      slog(LOG_ERR, "Could not update ob_campaign status to start. camp_uuid[%s], camp_name[%s]",
          json_string_value(json_object_get(j_camp, "uuid")),
          json_string_value(json_object_get(j_camp, "name"))
          );
    }
  }
  json_decref(j_camps);
  return;
}

/**
 * Check Stopping status campaign, and update to stop.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_campaign_stopping(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_camps;
  json_t* j_camp;
  int i;
  int size;
  int ret;

  j_camps = get_ob_campaigns_by_status(E_CAMP_STOPPING);
  if(j_camps == NULL) {
    // Nothing.
    return;
  }

  size = json_array_size(j_camps);
  for(i = 0; i < size; i++) {
    j_camp = json_array_get(j_camps, i);

    // check stoppable campaign
    ret = is_stoppable_campgain(j_camp);
    if(ret == false) {
      continue;
    }

    // update status to stop
    slog(LOG_NOTICE, "update ob_campaign status to stop. camp_uuid[%s], camp_name[%s]",
        json_string_value(json_object_get(j_camp, "uuid")),
        json_string_value(json_object_get(j_camp, "name"))
        );
    ret = update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOP);
    if(ret == false) {
      slog(LOG_ERR, "Could not update ob_campaign status to stop. camp_uuid[%s], camp_name[%s]",
        json_string_value(json_object_get(j_camp, "uuid")),
        json_string_value(json_object_get(j_camp, "name"))
        );
    }
  }

  json_decref(j_camps);

}

/**
 * Check Stopping status campaign, and update to stop.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_campaign_stopping_force(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
//  json_t* j_camps;
//  json_t* j_camp;
//  struct ao2_iterator iter;
//  rb_dialing* dialing;
//  const char* tmp_const;
//  int i;
//  int size;
//
//  j_camps = get_campaigns_by_status(E_CAMP_STOPPING_FORCE);
//  if(j_camps == NULL) {
//    // Nothing.
//    return;
//  }
//
//  // find dialing info
//  size = json_array_size(j_camps);
//  for(i = 0; i < size; i++) {
//    j_camp = json_array_get(j_camps, i);
//    slog(LOG_DEBUG, "Force stop campaign info. camp_uuid[%s], camp_name[%s]",
//        json_string_value(json_object_get(j_camp, "uuid")),
//        json_string_value(json_object_get(j_camp, "name"))
//        );
//
//    iter = rb_dialing_iter_init();
//    while(1) {
//      dialing = rb_dialing_iter_next(&iter);
//      if(dialing == NULL) {
//        break;
//      }
//
//      tmp_const = json_string_value(json_object_get(dialing->j_dialing, "camp_uuid"));
//      if(tmp_const == NULL) {
//        continue;
//      }
//
//      if(strcmp(tmp_const, json_string_value(json_object_get(j_camp, "uuid"))) != 0) {
//        continue;
//      }
//
//      // hang up the channel
//      slog(LOG_NOTICE, "Hangup channel. uuid[%s], channel[%s]",
//          dialing->uuid,
//          json_string_value(json_object_get(dialing->j_event, "channel"))
//          );
//      ami_cmd_hangup(json_string_value(json_object_get(dialing->j_event, "channel")), AST_CAUSE_NORMAL_CLEARING);
//    }
//    rb_dialing_iter_destroy(&iter);
//
//    // update status to stop
//    update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOP);
//  }
//  json_decref(j_camps);
}


///**
// * Check Stopping status campaign, and update to stop.
// * @param fd
// * @param event
// * @param arg
// */
//static void cb_campaign_schedule_stopping(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
//{
//  json_t* j_camps;
//  json_t* j_camp;
//  int i;
//  int size;
//  int ret;
//
//  j_camps = get_campaigns_by_status(E_CAMP_SCHEDULE_STOPPING);
//  if(j_camps == NULL) {
//    // Nothing.
//    return;
//  }
//
//  size = ast_json_array_size(j_camps);
//  for(i = 0; i < size; i++) {
//    j_camp = ast_json_array_get(j_camps, i);
//
//    // check stoppable campaign
//    ret = is_stoppable_campgain(j_camp);
//    if(ret == false) {
//      continue;
//    }
//
//    // update status to stop
//    slog(LOG_NOTICE, "Update campaign status to schedule stop. camp_uuid[%s], camp_name[%s]",
//        json_string_value(json_object_get(j_camp, "uuid")),
//        json_string_value(json_object_get(j_camp, "name"))
//        );
//    ret = update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_SCHEDULE_STOP);
//    if(ret == false) {
//      slog(LOG_ERR, "Could not update campaign status to schedule_stop. camp_uuid[%s], camp_name[%s]",
//        json_string_value(json_object_get(j_camp, "uuid")),
//        json_string_value(json_object_get(j_camp, "name"))
//        );
//    }
//  }
//
//  json_decref(j_camps);
//
//}

static void cb_check_dialing_end(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
//  struct ao2_iterator iter;
//  rb_dialing* dialing;
//  json_t* j_tmp;
//  int ret;
//  char* timestamp;
//
//  iter = rb_dialing_iter_init();
//  while(1) {
//    dialing = rb_dialing_iter_next(&iter);
//    if(dialing == NULL) {
//      break;
//    }
//
//    if(dialing->status != E_DIALING_HANGUP) {
//      continue;
//    }
//
//    // create dl_list for update
//    timestamp = get_utc_timestamp();
//    j_tmp = json_pack("{s:s, s:i, s:O, s:O, s:O, s:s}",
//        "uuid",         json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")),
//        "status",         E_DL_IDLE,
//        "dialing_uuid",     json_null(),
//        "dialing_camp_uuid",  json_null(),
//        "dialing_plan_uuid",  json_null(),
//        "tm_last_hangup",      timestamp
//        );
//    json_object_set_new(j_tmp, "res_hangup", json_incref(json_object_get(dialing->j_dialing, "res_hangup")));
//    json_object_set_new(j_tmp, "res_dial", json_incref(json_object_get(dialing->j_dialing, "res_dial")));
//    sfree(timestamp);
//    if(j_tmp == NULL) {
//      slog(LOG_ERR, "Could not create update dl_list json. dl_list_uuid[%s], res_hangup[%lld], res_dial[%lld]",
//          json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")),
//          json_integer_value(json_object_get(dialing->j_dialing, "res_hangup")),
//          json_integer_value(json_object_get(dialing->j_dialing, "res_dial"))
//          );
//    }
//
//    // update dl_list
//    ret = update_dl_list(j_tmp);
//    json_decref(j_tmp);
//    if(ret == false) {
//      slog(LOG_WARNING, "Could not update dialing result. dialing_uuid[%s], dl_list_uuid[%s]",
//          dialing->uuid, json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")));
//      continue;
//    }
//
//    // create result data
//    j_tmp = create_json_for_dl_result(dialing);
//    slog(LOG_DEBUG, "Check result value. dial_channel[%s], dial_addr[%s], dial_index[%lld], dial_trycnt[%lld], dial_timeout[%lld], dial_type[%lld], dial_exten[%s], res_dial[%lld], res_hangup[%lld], res_hangup_detail[%s]",
//
//        // dial
//        json_string_value(json_object_get(j_tmp, "dial_channel")),
//        json_string_value(json_object_get(j_tmp, "dial_addr")),
//        json_integer_value(json_object_get(j_tmp, "dial_index")),
//        json_integer_value(json_object_get(j_tmp, "dial_trycnt")),
//        json_integer_value(json_object_get(j_tmp, "dial_timeout")),
//        json_integer_value(json_object_get(j_tmp, "dial_type")),
//        json_string_value(json_object_get(j_tmp, "dial_exten")),
//
//        // result
//        json_integer_value(json_object_get(j_tmp, "res_dial")),
//        json_integer_value(json_object_get(j_tmp, "res_hangup")),
//        json_string_value(json_object_get(j_tmp, "res_hangup_detail"))
//        );
//
////    db_insert("dl_result", j_tmp);
//    ret = write_result_json(j_tmp);
//    json_decref(j_tmp);
//    if(ret == false) {
//      slog(LOG_ERR, "Could not write result correctly.");
//      continue;
//    }
//
//    rb_dialing_destory(dialing);
//    slog(LOG_DEBUG, "Destroyed dialing info.");
//
//  }
//  ao2_iterator_destroy(&iter);
//
//  return;
}

static void cb_check_dialing_error(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  // todo:
//  slog(LOG_ERR, "Need to fix.");
//  struct ao2_iterator iter;
//  rb_dialing* dialing;
//  json_t* j_tmp;
//  int ret;
//  char* timestamp;
//
//  iter = rb_dialing_iter_init();
//  while(1) {
//    dialing = rb_dialing_iter_next(&iter);
//    if(dialing == NULL) {
//      break;
//    }
//
//    if(dialing->status != E_DIALING_ERROR) {
//      continue;
//    }
//
//
//    // create dl_list for update
//    timestamp = get_utc_timestamp();
//    j_tmp = json_pack("{s:s, s:i, s:O, s:O, s:O, s:s}",
//        "uuid",         json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")),
//        "status",         E_DL_IDLE,
//        "dialing_uuid",     json_null(),
//        "dialing_camp_uuid",  json_null(),
//        "dialing_plan_uuid",  json_null(),
//        "tm_last_hangup",      timestamp
//        );
//    json_object_set_new(j_tmp, "res_hangup", json_incref(json_object_get(dialing->j_dialing, "res_hangup")));
//    json_object_set_new(j_tmp, "res_dial", json_incref(json_object_get(dialing->j_dialing, "res_dial")));
//    sfree(timestamp);
//    if(j_tmp == NULL) {
//      slog(LOG_ERR, "Could not create update dl_list json. dl_list_uuid[%s], res_hangup[%lld], res_dial[%lld]",
//          json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")),
//          json_integer_value(json_object_get(dialing->j_dialing, "res_hangup")),
//          json_integer_value(json_object_get(dialing->j_dialing, "res_dial"))
//          );
//    }
//
//    // update dl_list
//    ret = update_dl_list(j_tmp);
//    json_decref(j_tmp);
//    if(ret == false) {
//      slog(LOG_WARNING, "Could not update dialing result. dialing_uuid[%s], dl_list_uuid[%s]",
//          dialing->uuid, json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")));
//      continue;
//    }
//
//    // create result data
//    j_tmp = create_json_for_dl_result(dialing);
//    slog(LOG_DEBUG, "Check result value. dial_channel[%s], dial_addr[%s], dial_index[%lld], dial_trycnt[%lld], dial_timeout[%lld], dial_type[%lld], dial_exten[%s], res_dial[%lld], res_hangup[%lld], res_hangup_detail[%s]",
//
//        // dial
//        json_string_value(json_object_get(j_tmp, "dial_channel")),
//        json_string_value(json_object_get(j_tmp, "dial_addr")),
//        json_integer_value(json_object_get(j_tmp, "dial_index")),
//        json_integer_value(json_object_get(j_tmp, "dial_trycnt")),
//        json_integer_value(json_object_get(j_tmp, "dial_timeout")),
//        json_integer_value(json_object_get(j_tmp, "dial_type")),
//        json_string_value(json_object_get(j_tmp, "dial_exten")),
//
//        // result
//        json_integer_value(json_object_get(j_tmp, "res_dial")),
//        json_integer_value(json_object_get(j_tmp, "res_hangup")),
//        json_string_value(json_object_get(j_tmp, "res_hangup_detail"))
//        );
//
//    ret = write_result_json(j_tmp);
//    json_decref(j_tmp);
//
//    rb_dialing_destory(dialing);
//    slog(LOG_DEBUG, "Destroyed!");
//
//  }
//  ao2_iterator_destroy(&iter);
//
//  return;
}

/**
 * Check ended campaign. If the campaign is end-able, set the status to STOPPING.
 * \param fd
 * \param event
 * \param arg
 */
static void cb_check_campaign_end(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_camps;
  json_t* j_camp;
  json_t* j_plan;
  json_t* j_dlma;
  int i;
  int size;
  int ret;

  j_camps = get_ob_campaigns_by_status(E_CAMP_START);
  size = json_array_size(j_camps);
  for(i = 0; i < size; i++) {
    j_camp = json_array_get(j_camps, i);
    if(j_camp == NULL) {
      continue;
    }

    j_plan = get_ob_plan(json_string_value(json_object_get(j_camp, "plan")));
    if(j_plan == NULL) {
      update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
      continue;
    }

    j_dlma = get_ob_dlma(json_string_value(json_object_get(j_camp, "dlma")));
    if(j_dlma == NULL) {
      update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
      json_decref(j_plan);
      continue;
    }

    // check end-able.
    ret = is_endable_ob_plan(j_plan);
    ret &= is_endable_dl_list(j_dlma, j_plan);
    if(ret == true) {
      slog(LOG_NOTICE, "The campaign ended. Stopping campaign. uuid[%s], name[%s]",
          json_string_value(json_object_get(j_camp, "uuid")),
          json_string_value(json_object_get(j_camp, "name"))
          );
      update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    }
    json_decref(j_plan);
    json_decref(j_dlma);
  }
  json_decref(j_camps);
  return;
}

/**
 * Check campaign schedule. If the campaign is start-able, set the status to STARTING.
 * \param fd
 * \param event
 * \param arg
 */
static void cb_check_campaign_schedule_start(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_camps;
  json_t* j_camp;
  int size;
  int i;
  int ret;

  j_camps = get_campaigns_schedule_start();
  size = json_array_size(j_camps);
  for(i = 0; i < size; i++) {
    j_camp = json_array_get(j_camps, i);
    if(j_camp == NULL) {
      continue;
    }

    slog(LOG_NOTICE, "update ob_campaign status to starting by scheduling. camp_uuid[%s], camp_name[%s]",
        json_string_value(json_object_get(j_camp, "uuid"))? : "",
        json_string_value(json_object_get(j_camp, "name"))? : ""
        );
    ret = update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STARTING);
    if(ret == false) {
      slog(LOG_ERR, "Could not update ob_campaign status to starting. camp_uuid[%s], camp_name[%s]",
          json_string_value(json_object_get(j_camp, "uuid"))? : "",
          json_string_value(json_object_get(j_camp, "name"))? : ""
          );
    }
  }

  json_decref(j_camps);
}

/**
 * Check campaign schedule. If the campaign is end-able, set the status to STOPPING.
 * \param fd
 * \param event
 * \param arg
 */
static void cb_check_campaign_schedule_end(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_camps;
  json_t* j_camp;
  int size;
  int i;
  int ret;

  j_camps = get_campaigns_schedule_end();
  size = json_array_size(j_camps);
  for(i = 0; i < size; i++) {
    j_camp = json_array_get(j_camps, i);
    if(j_camp == NULL) {
      continue;
    }

    slog(LOG_NOTICE, "Update ob_campaign status to stopping by scheduling. camp_uuid[%s], camp_name[%s]",
        json_string_value(json_object_get(j_camp, "uuid")),
        json_string_value(json_object_get(j_camp, "name"))
        );
    ret = update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    if(ret == false) {
      slog(LOG_ERR, "Could not update ob_campaign status to schedule_stopping. camp_uuid[%s], camp_name[%s]",
          json_string_value(json_object_get(j_camp, "uuid")),
          json_string_value(json_object_get(j_camp, "name"))
          );
    }
  }

  json_decref(j_camps);
}



/**
 *
 * @param j_camp
 * @param j_plan
 */
static void dial_desktop(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma)
{
  return;
}

/**
 *
 * @param j_camp
 * @param j_plan
 */
static void dial_power(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma)
{
  return;
}

/**
 *  Make a call by predictive algorithms.
 *  Currently, just consider ready agent only.
 * @param j_camp  campaign info
 * @param j_plan  plan info
 * @param j_dlma  dial list master info
 */
static void dial_predictive(json_t* j_camp, json_t* j_plan, json_t* j_dlma, json_t* j_dest)
{
  int ret;
  json_t* j_dl_list;
  json_t* j_dial;
  json_t* j_dialing;
  E_DESTINATION_TYPE dial_type;

  // get dl_list info to dial.
  j_dl_list = get_dl_available_predictive(j_dlma, j_plan);
  if(j_dl_list == NULL) {
    // No available list
//    slog(LOG_VERBOSE, "No more dialing list. stopping the campaign.");
//    update_campaign_info_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    return;
  }

  // check available outgoing call.
  ret = check_dial_avaiable_predictive(j_camp, j_plan, j_dlma, j_dest);
  if(ret == -1) {
    // something was wrong. stop the campaign.
    update_ob_campaign_status(json_string_value(json_object_get(j_camp, "uuid")), E_CAMP_STOPPING);
    json_decref(j_dl_list);
    return;
  }
  else if(ret == 0) {
    // Too much calls already outgoing.
    json_decref(j_dl_list);
    return;
  }

  // creating dialing info
  j_dial = create_dial_info(j_plan, j_dl_list, j_dest);
  if(j_dial == NULL) {
    json_decref(j_dl_list);
    slog(LOG_DEBUG, "Could not create dialing info.");
    return;
  }
  slog(LOG_NOTICE, "Originating. camp_uuid[%s], camp_name[%s], channel[%s], chan_id[%s], timeout[%lld], dial_index[%lld], dial_trycnt[%lld], dial_type[%lld]",
      json_string_value(json_object_get(j_camp, "uuid")),
      json_string_value(json_object_get(j_camp, "name")),
      json_string_value(json_object_get(j_dial, "dial_channel")),
      json_string_value(json_object_get(j_dial, "channelid")),
      json_integer_value(json_object_get(j_dial, "dial_timeout")),
      json_integer_value(json_object_get(j_dial, "dial_index")),
      json_integer_value(json_object_get(j_dial, "dial_trycnt")),
      json_integer_value(json_object_get(j_dial, "dial_type"))
      );

  // create ob_dialing
  j_dialing = create_ob_dialing(
      json_string_value(json_object_get(j_dial, "channelid")),
      j_camp,
      j_plan,
      j_dlma,
      j_dest,
      j_dl_list,
      j_dial
      );
  json_decref(j_dl_list);
  json_decref(j_dial);
  if(j_dialing == NULL) {
    slog(LOG_WARNING, "Could not create dialing info.");
    return;
  }

  // update dl list using dialing info
  ret = update_dl_list_after_create_dialing_info(j_dialing);
  if(ret == false) {
    json_decref(j_dialing);
    clear_dl_list_dialing(json_string_value(json_object_get(j_dialing, "uuid_dl_list")));
    slog(LOG_ERR, "Could not update dial list info.");
    return;
  }
  slog(LOG_DEBUG, "Updated ob_dl after creating dialing info.");

  // dial to customer
  dial_type = json_integer_value(json_object_get(j_dialing, "dial_type"));
  switch(dial_type) {
    case DESTINATION_EXTEN: {
      ret = originate_to_exten(j_dialing);
    }
    break;

    case DESTINATION_APPLICATION: {
      ret = originate_to_application(j_dialing);
    }
    break;

    default: {
      slog(LOG_ERR, "Unsupported dialing type.");
      ret = false;
    }
    break;
  }
  slog(LOG_DEBUG, "Break point.");

  if(ret == false) {
    slog(LOG_WARNING, "Originating has been failed.");
    clear_dl_list_dialing(json_string_value(json_object_get(j_dialing, "uuid_dl_list")));
    update_ob_dialing_status(json_string_value(json_object_get(j_dialing, "uuid")), E_DIALING_ORIGINATE_REQUEST_FAILED);
    json_decref(j_dialing);
    return;
  }

  ret = update_ob_dialing_status(
      json_string_value(json_object_get(j_dialing, "uuid")),
      E_DIALING_ORIGINATE_REQUEST
      );
  json_decref(j_dialing);
  return;
}

/**
 *
 * @param j_camp
 * @param j_plan
 */
static void dial_robo(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma)
{
  return;
}

/**
 *  Redirect call to other dialplan.
 * @param j_camp  campaign info
 * @param j_plan  plan info
 * @param j_dlma  dial list master info
 */
static void dial_redirect(const json_t* j_camp, const json_t* j_plan, const json_t* j_dlma)
{
  return;
}




/**
 * Return dialing availability.
 * todo: need something more here.. currently, just compare dial numbers..
 * If can dialing returns true, if not returns false.
 * @param j_camp
 * @param j_plan
 * @return 1:OK, 0:NO, -1:ERROR
 */
static int check_dial_avaiable_predictive(
    json_t* j_camp,
    json_t* j_plan,
    json_t* j_dlma,
    json_t* j_dest
    )
{
  int cnt_current_dialing;
  int plan_service_level;
  int cnt_avail;
  int ret;

  // get available destination count
  cnt_avail = get_ob_destination_available_count(j_dest);
  if(cnt_avail == DEF_DESTINATION_AVAIL_CNT_UNLIMITED) {
    slog(LOG_DEBUG, "Available destination count is unlimited. cnt[%d]", cnt_avail);
    return 1;
  }
  slog(LOG_DEBUG, "Available destination count. cnt[%d]", cnt_avail);

  // get service level
  plan_service_level = json_integer_value(json_object_get(j_plan, "service_level"));
  if(plan_service_level < 0) {
    plan_service_level = 0;
  }
  slog(LOG_DEBUG, "Service level. level[%d]", plan_service_level);

  // get current dialing count
  cnt_current_dialing = get_ob_dialing_count_by_camp_uuid(json_string_value(json_object_get(j_camp, "uuid")));
  if(cnt_current_dialing == -1) {
    slog(LOG_ERR, "Could not get current dialing count info. camp_uuid[%s]",
        json_string_value(json_object_get(j_camp, "uuid"))
        );
    return -1;
  }
  slog(LOG_DEBUG, "Current dialing count. count[%d]", cnt_current_dialing);

  ret = (cnt_avail + plan_service_level) - cnt_current_dialing;

  if(ret <= 0) {
    return 0;
  }

  return 1;
}

//static bool write_result_json(json_t* j_res)
//{
//  FILE* fp;
//  json_t* j_general;
//  const char* filename;
//  char* tmp;
//
//  if(j_res == NULL) {
//    slog(LOG_ERR, "Wrong input parameter.");
//    return false;
//  }
//
//  // open json file
//  j_general = json_object_get(g_app->j_conf, "general");
//  filename = json_string_value(json_object_get(j_general, "result_filename"));
//  if(filename == NULL) {
//    slog(LOG_ERR, "Could not get option value. option[%s]", "result_filename");
//    return false;
//  }
//
//  // open file
//  fp = fopen(filename, "a");
//  if(fp == NULL) {
//    slog(LOG_ERR, "Could not open result file. filename[%s], err[%s]",
//        filename, strerror(errno)
//        );
//    return false;
//  }
//
//  tmp = json_dumps(j_res, JSON_ENCODE_ANY);
//  if(tmp == NULL) {
//    slog(LOG_ERR, "Could not get result string to the file. filename[%s], err[%s]",
//        filename, strerror(errno)
//        );
//    fclose(fp);
//    return false;
//  }
//
//  fprintf(fp, "%s", tmp);
//  sfree(tmp);
//  fclose(fp);
//
//  slog(LOG_VERBOSE, "Result write succeed.");
//
//  return true;
//}



