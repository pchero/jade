/*
 * dl_handler.c
 *
 *  Created on: Nov 29, 2015
 *    Author: pchero
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <jansson.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "db_ctx_handler.h"
#include "ob_event_handler.h"
#include "ob_campaign_handler.h"
#include "ob_dl_handler.h"
#include "ob_destination_handler.h"
#include "ob_plan_handler.h"
#include "ob_dlma_handler.h"

extern app* g_app;
extern db_ctx_t* g_db_ob;

#define DEF_DL_STATUS   E_DL_STATUS_IDLE

static json_t* get_ob_dl_use(const char* uuid, E_USE use);
static json_t* get_dls_uuid_available(json_t* j_dlma, json_t* j_plan, int count);
static json_t* create_ob_dl_default(void);
static json_t* get_ob_dls_uuid_count(int count);

static json_t* get_deleted_ob_dl(const char* uuid);
static char* create_chan_addr_for_dial(json_t* j_plan, json_t* j_dl_list, int dial_num_point);
static char* get_dial_number(json_t* j_dlist, const int cnt);
static json_t* create_dial_dl_info(json_t* j_dl_list, json_t* j_plan);
static bool check_more_dl_list(json_t* j_dlma, json_t* j_plan);
static bool is_over_retry_delay(json_t* j_dlma, json_t* j_dl, json_t* j_plan);
static json_t* get_ob_dl_available(json_t* j_dlma, json_t* j_plan);


/*!
 * \brief Internal control frame subtype field values.
 *
 * \warning
 * IAX2 sends these values out over the wire.  To prevent future
 * incompatibilities, pick the next value in the enum from whatever
 * is on the current trunk.  If you lose the merge race you need to
 * fix the previous branches to match what is on trunk.  In addition
 * you need to change chan_iax2 to explicitly allow the control
 * frame over the wire if it makes sense for the frame to be passed
 * to another Asterisk instance.
 */
enum ast_control_frame_type {
  AST_CONTROL_HANGUP = 1,     /*!< Other end has hungup */
  AST_CONTROL_RING = 2,     /*!< Local ring */
  AST_CONTROL_RINGING = 3,    /*!< Remote end is ringing */
  AST_CONTROL_ANSWER = 4,     /*!< Remote end has answered */
  AST_CONTROL_BUSY = 5,     /*!< Remote end is busy */
  AST_CONTROL_TAKEOFFHOOK = 6,  /*!< Make it go off hook */
  AST_CONTROL_OFFHOOK = 7,    /*!< Line is off hook */
  AST_CONTROL_CONGESTION = 8,   /*!< Congestion (circuits busy) */
  AST_CONTROL_FLASH = 9,      /*!< Flash hook */
  AST_CONTROL_WINK = 10,      /*!< Wink */
  AST_CONTROL_OPTION = 11,    /*!< Set a low-level option */
  AST_CONTROL_RADIO_KEY = 12,   /*!< Key Radio */
  AST_CONTROL_RADIO_UNKEY = 13, /*!< Un-Key Radio */
  AST_CONTROL_PROGRESS = 14,    /*!< Indicate PROGRESS */
  AST_CONTROL_PROCEEDING = 15,  /*!< Indicate CALL PROCEEDING */
  AST_CONTROL_HOLD = 16,      /*!< Indicate call is placed on hold */
  AST_CONTROL_UNHOLD = 17,    /*!< Indicate call is left from hold */
  AST_CONTROL_VIDUPDATE = 18,   /*!< Indicate video frame update */
  _XXX_AST_CONTROL_T38 = 19,    /*!< T38 state change request/notification \deprecated This is no longer supported. Use AST_CONTROL_T38_PARAMETERS instead. */
  AST_CONTROL_SRCUPDATE = 20,   /*!< Indicate source of media has changed */
  AST_CONTROL_TRANSFER = 21,    /*!< Indicate status of a transfer request */
  AST_CONTROL_CONNECTED_LINE = 22,/*!< Indicate connected line has changed */
  AST_CONTROL_REDIRECTING = 23, /*!< Indicate redirecting id has changed */
  AST_CONTROL_T38_PARAMETERS = 24,/*!< T38 state change request/notification with parameters */
  AST_CONTROL_CC = 25,      /*!< Indication that Call completion service is possible */
  AST_CONTROL_SRCCHANGE = 26,   /*!< Media source has changed and requires a new RTP SSRC */
  AST_CONTROL_READ_ACTION = 27, /*!< Tell ast_read to take a specific action */
  AST_CONTROL_AOC = 28,     /*!< Advice of Charge with encoded generic AOC payload */
  AST_CONTROL_END_OF_Q = 29,    /*!< Indicate that this position was the end of the channel queue for a softhangup. */
  AST_CONTROL_INCOMPLETE = 30,  /*!< Indication that the extension dialed is incomplete */
  AST_CONTROL_MCID = 31,      /*!< Indicate that the caller is being malicious. */
  AST_CONTROL_UPDATE_RTP_PEER = 32, /*!< Interrupt the bridge and have it update the peer */
  AST_CONTROL_PVT_CAUSE_CODE = 33, /*!< Contains an update to the protocol-specific cause-code stored for branching dials */
  AST_CONTROL_MASQUERADE_NOTIFY = 34, /*!< A masquerade is about to begin/end. (Never sent as a frame but directly with ast_indicate_data().) */

  /*
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   *
   * IAX2 sends these values out over the wire.  To prevent future
   * incompatibilities, pick the next value in the enum from whatever
   * is on the current trunk.  If you lose the merge race you need to
   * fix the previous branches to match what is on trunk.  In addition
   * you need to change chan_iax2 to explicitly allow the control
   * frame over the wire if it makes sense for the frame to be passed
   * to another Asterisk instance.
   *
   * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
   */

  /* Control frames used to manipulate a stream on a channel. The values for these
   * must be greater than the allowed value for a 8-bit char, so that they avoid
   * conflicts with DTMF values. */
  AST_CONTROL_STREAM_STOP = 1000,   /*!< Indicate to a channel in playback to stop the stream */
  AST_CONTROL_STREAM_SUSPEND = 1001,  /*!< Indicate to a channel in playback to suspend the stream */
  AST_CONTROL_STREAM_RESTART = 1002,  /*!< Indicate to a channel in playback to restart the stream */
  AST_CONTROL_STREAM_REVERSE = 1003,  /*!< Indicate to a channel in playback to rewind */
  AST_CONTROL_STREAM_FORWARD = 1004,  /*!< Indicate to a channel in playback to fast forward */
  /* Control frames to manipulate recording on a channel. */
  AST_CONTROL_RECORD_CANCEL = 1100, /*!< Indicated to a channel in record to stop recording and discard the file */
  AST_CONTROL_RECORD_STOP = 1101, /*!< Indicated to a channel in record to stop recording */
  AST_CONTROL_RECORD_SUSPEND = 1102,  /*!< Indicated to a channel in record to suspend/unsuspend recording */
  AST_CONTROL_RECORD_MUTE = 1103, /*!< Indicated to a channel in record to mute/unmute (i.e. write silence) recording */
};

/**
 * Get dl_list for predictive dialing.
 * \param j_dlma
 * \param j_plan
 * \return
 */
json_t* get_dl_available_for_dial(json_t* j_dlma, json_t* j_plan)
{
  json_t* j_dl;
  int ret;

  if((j_dlma == NULL) || (j_plan == NULL)) {
    slog(LOG_WARNING, "Wrong input parameters.");
    return NULL;
  }

  // get available dl.
  j_dl = get_ob_dl_available(j_dlma, j_plan);
  if(j_dl == NULL) {
    return NULL;
  }

  // check retry delay
  ret = is_over_retry_delay(j_dlma, j_dl, j_plan);
  if(ret == false) {
    json_decref(j_dl);
    return NULL;
  }

  return j_dl;
}

static bool check_more_dl_list(json_t* j_dlma, json_t* j_plan)
{
  json_t* j_res;
  int ret;
  char* sql;

  asprintf(&sql, "select uuid from `%s` where ("
      "(number_1 is not null and trycnt_1 < %lld)"
      " or (number_2 is not null and trycnt_2 < %lld)"
      " or (number_3 is not null and trycnt_3 < %lld)"
      " or (number_4 is not null and trycnt_4 < %lld)"
      " or (number_5 is not null and trycnt_5 < %lld)"
      " or (number_6 is not null and trycnt_6 < %lld)"
      " or (number_7 is not null and trycnt_7 < %lld)"
      " or (number_8 is not null and trycnt_8 < %lld)"
      ")"
      " and res_dial != %d"
      ";",
      json_string_value(json_object_get(j_dlma, "dl_table")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_1")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_2")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_3")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_4")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_5")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_6")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_7")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_8")),
      AST_CONTROL_ANSWER
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    return false;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
//  ast_log(LOG_DEBUG, "Get dial records. uuid[%s]", ast_json_string_get(ast_json_object_get(j_res, "uuid")));
  if(j_res == NULL) {
    return false;
  }
  json_decref(j_res);

  return true;
}

/**
 *
 * @param uuid
 */
void clear_dl_list_dialing(const char* uuid)
{
  json_t* j_tmp;
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired clear_dl_list_dialing. uuid[%s]", uuid);

  j_tmp = json_pack("{s:i, s:s, s:o, s:o, s:o}",
      "status",             E_DL_STATUS_IDLE,
      "uuid",               uuid,
      "dialing_uuid",       json_null(),
      "dialing_camp_uuid",  json_null(),
      "dialing_plan_uuid",  json_null()
      );
  j_res = update_ob_dl(j_tmp);
  json_decref(j_tmp);
  json_decref(j_res);

  return;
}

/**
 * Update dl list info.
 * @param j_dlinfo
 * @return
 */
json_t* update_ob_dl(json_t* j_dl)
{
  char* sql;
  int ret;
  char* tmp;
  char* uuid;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_dl == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_tmp = json_deep_copy(j_dl);
  if(j_tmp == NULL) {
    return NULL;
  }

  tmp_const = json_string_value(json_object_get(j_tmp, "uuid"));
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get uuid info.");
    json_decref(j_tmp);
    return NULL;
  }
  uuid = strdup(tmp_const);

  tmp = db_ctx_get_update_str(j_tmp);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not get update sql.");
    sfree(uuid);
    json_decref(j_tmp);
    return NULL;
  }
  json_decref(j_tmp);

  asprintf(&sql, "update ob_dl_list set %s where uuid = \"%s\";",
      tmp, uuid
      );
  sfree(tmp);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not update ob_dl_list info.");
    return NULL;
  }

  slog(LOG_DEBUG, "Getting updated ob_dl info. uuid[%s]", uuid);
  j_tmp = get_ob_dl(uuid);
  sfree(uuid);
  if(j_tmp == NULL) {
    slog(LOG_WARNING, "Could not get updated ob_dl info.");
    return NULL;
  }

  return j_tmp;
}

/**
 * Return the current dialing count.
 * @param camp_uuid
 * @param dl_table
 * @return
 */
int get_current_dialing_dl_cnt(const char* camp_uuid, const char* dl_table)
{
  int ret;
  char* sql;
  json_t* j_tmp;

  if((camp_uuid == NULL) || (dl_table == NULL)) {
    slog(LOG_ERR, "Invalid input parameters.");
    return -1;
  }

  asprintf(&sql, "select count(*) from `%s` where dialing_camp_uuid = \"%s\" and status = \"%s\";",
      dl_table, camp_uuid, "dialing"
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get dialing count.");
    return 0;
  }

  j_tmp = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_tmp == NULL) {
    // shouldn't be reach to here.
    slog(LOG_ERR, "Could not get dialing count.");
    return false;
  }

  ret = json_integer_value(json_object_get(j_tmp, "count(*)"));
  json_decref(j_tmp);

  return ret;
}

/**
 * Get dial number index
 * @param j_dl_list
 * @param j_plan
 * @return
 */
int get_dial_num_point(json_t* j_dl_list, json_t* j_plan)
{
  int i;
  int dial_num_point;
  int cur_trycnt;
  int max_trycnt;
  char* tmp;
  const char* tmp_const;

  // get dial number
  dial_num_point = -1;
  for(i = 1; i < 9; i++) {
    asprintf(&tmp, "number_%d", i);
    tmp_const = json_string_value(json_object_get(j_dl_list, tmp));
    sfree(tmp);
    if(tmp_const == NULL) {
      // No number set.
      continue;
    }

    asprintf(&tmp, "trycnt_%d", i);
    cur_trycnt = json_integer_value(json_object_get(j_dl_list, tmp));
    sfree(tmp);

    asprintf(&tmp, "max_retry_cnt_%d", i);
    max_trycnt = json_integer_value(json_object_get(j_plan, tmp));
    sfree(tmp);

    if(cur_trycnt < max_trycnt) {
      dial_num_point = i;
      break;
    }
  }

  return dial_num_point;
}

/**
 * Get dial try count for this time.
 * @param j_dl_list
 * @param dial_num_point
 * @return
 */
int get_dial_try_cnt(json_t* j_dl_list, int dial_num_point)
{
  int cur_trycnt;
  char* tmp;

  asprintf(&tmp, "trycnt_%d", dial_num_point);
  if(json_object_get(j_dl_list, tmp) == NULL) {
    return -1;
  }

  cur_trycnt = json_integer_value(json_object_get(j_dl_list, tmp));
  sfree(tmp);
  cur_trycnt++;   // for this time.

  return cur_trycnt;
}

json_t* get_ob_dls_uuid_by_dlma_count(const char* dlma_uuid, int count)
{
  char* sql;
  int ret;
  json_t* j_res_tmp;
  json_t* j_res;
  json_t* j_tmp;
  char* dl_table;

  if((dlma_uuid == NULL) || (count <= 0)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get dl_table
  dl_table = get_ob_dlma_table_name(dlma_uuid);
  if(dl_table == NULL) {
    slog(LOG_NOTICE, "Could not get correct dl_table name. dlma_uuid[%s]", dlma_uuid);
    return NULL;
  }

  asprintf(&sql, "select uuid from `%s` where in_use=%d limit %d;",
      dl_table,
      E_USE_OK,
      count
      );
  sfree(dl_table);

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get dial list info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ob);
    if(j_tmp == NULL) {
      break;
    }

    j_res_tmp = json_pack("{s:s}",
        "uuid", json_string_value(json_object_get(j_tmp, "uuid"))
        );
    json_decref(j_tmp);
    if(j_res_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_res_tmp);
  }
  db_ctx_free(g_db_ob);

  return j_res;
}

/**
 * Get dl_list from database.
 * @param j_dlma
 * @param j_plan
 * @return
 */
json_t* get_ob_dls_by_dlma_count(const char* dlma_uuid, int count)
{
  json_t* j_res;
  json_t* j_tmp;
  json_t* j_uuids;
  json_t* j_val;
  unsigned int idx;
  const char* uuid;

  if((dlma_uuid == NULL) || (count <= 0)) {
    return NULL;
  }

  j_uuids = get_ob_dls_uuid_by_dlma_count(dlma_uuid, count);

  j_res = json_array();
  json_array_foreach(j_uuids, idx, j_val) {
    uuid = json_string_value(json_object_get(j_val, "uuid"));
    if(uuid == NULL) {
      continue;
    }

    j_tmp = get_ob_dl(uuid);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_uuids);

  return j_res;
}

static json_t* get_ob_dls_uuid_count(int count)
{
  char* sql;
  int ret;
  json_t* j_res;
  json_t* j_tmp;
  const char* uuid;

  if(count <= 0) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  asprintf(&sql, "select uuid from ob_dl_list where in_use=%d limit %d;",
      E_USE_OK,
      count
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_dl_list info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ob);
    if(j_tmp == NULL) {
      break;
    }

    uuid = json_string_value(json_object_get(j_tmp, "uuid"));
    if(uuid == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res, json_string(uuid));
    json_decref(j_tmp);
  }
  db_ctx_free(g_db_ob);

  return j_res;
}

/**
 * Get dl_list from database.
 * @param j_dlma
 * @param j_plan
 * @return
 */
json_t* get_ob_dls_by_count(int count)
{
  json_t* j_uuids;
  json_t* j_val;
  json_t* j_res;
  json_t* j_tmp;
  unsigned int idx;
  const char* uuid;

  j_uuids = get_ob_dls_uuid_count(count);

  j_res = json_array();
  json_array_foreach(j_uuids, idx, j_val) {
    uuid = json_string_value(j_val);
    if(uuid == NULL) {
      continue;
    }

    j_tmp = get_ob_dl(uuid);
    if(j_tmp == NULL) {
      continue;
    }

    json_array_append_new(j_res, j_tmp);
  }
  json_decref(j_uuids);

  return j_res;
}

/**
 *
 * @param status
 * @return
 */
json_t* get_ob_dls_by_status(E_DL_STATUS_T status)
{
  json_t* j_res;
  json_t* j_tmp;
  char* sql;
  int ret;

  slog(LOG_DEBUG, "Fired get_ob_dls_by_status. status[%d]", status);

  asprintf(&sql, "select * from ob_dl_list where status=%d and in_use=%d;", status, E_USE_OK);

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_dl_list info. status[%d]", status);
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ob);
    if(j_tmp == NULL) {
      break;
    }

    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(g_db_ob);

  return j_res;
}

/**
 * Get dial list array of invalid status.
 *
 * @return
 */
json_t* get_ob_dls_error(void)
{
  int ret;
  json_t* j_res;
  json_t* j_tmp;
  char* sql;

  asprintf(&sql, "select ob_dl_list.* from ob_dl_list"
      " inner join ob_dialing on ob_dl_list.uuid != ob_dialing.uuid_dl_list"
      " where ob_dl_list.status != %d;",
      E_DL_STATUS_IDLE
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_dl_list info for error.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ob);
    if(j_tmp == NULL) {
      break;
    }

    json_array_append_new(j_res, j_tmp);
  }
  db_ctx_free(g_db_ob);

  return j_res;
}

static json_t* get_ob_dl_use(const char* uuid, E_USE use)
{
  char* sql;
  json_t* j_res;
  int ret;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Invalid input parameters.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ob_dl_use. uuid[%s], use[%d]", uuid, use);

  asprintf(&sql, "select * from ob_dl_list where uuid=\"%s\" and in_use=%d;", uuid, use);

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_dl_list info. uuid[%s], use[%d]", uuid, use);
    return NULL;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);

  return j_res;
}

json_t* get_ob_dl(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_ob_dl. uuid[%s]", uuid);

  j_res = get_ob_dl_use(uuid, E_USE_OK);
  if(j_res == NULL) {
    slog(LOG_DEBUG, "Could not find dl info. uuid[%s]", uuid);
  }

  return j_res;
}

int get_ob_dl_count_by_dlma_uuid(const char* dlma_uuid)
{
  int ret;
  char* sql;
  char* dl_table;
  json_t* j_res;

  if(dlma_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 0;
  }
  slog(LOG_DEBUG, "Fired get_ob_dl_count_by_dlma_uuid. dlma_uuid[%s]", dlma_uuid);

  dl_table = get_ob_dlma_table_name(dlma_uuid);
  if(dl_table == NULL) {
    slog(LOG_ERR, "Could not get correct dlma table name. dlma_uuid[%s]", dlma_uuid);
    return 0;
  }

  asprintf(&sql, "select count(*) from %s where in_use=%d;",
      dl_table, E_USE_OK
      );
  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);

  ret = json_integer_value(json_object_get(j_res, "count(*)"));
  json_decref(j_res);

  return ret;
}

int get_ob_dl_list_cnt_total(json_t* j_dlma)
{
  int ret;

  if(j_dlma == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return -1;
  }
  slog(LOG_DEBUG, "Fired get_ob_dl_list_cnt_total.");

  ret = get_ob_dl_count_by_dlma_uuid(json_string_value(json_object_get(j_dlma, "uuid")));

  return ret;
}

/**
 * Get finished list count.
 * \param j_dlma
 * \param j_plan
 * \return
 */
int get_ob_dl_list_cnt_finshed(json_t* j_dlma, json_t* j_plan)
{
  int ret;
  char* sql;
  json_t* j_res;

  asprintf(&sql, "select count(*)"
      " from `%s` where "
      "("
      " (number_1 is not null and trycnt_1 >= %lld)"
      " or (number_2 is not null and trycnt_2 >= %lld)"
      " or (number_3 is not null and trycnt_3 >= %lld)"
      " or (number_4 is not null and trycnt_4 >= %lld)"
      " or (number_5 is not null and trycnt_5 >= %lld)"
      " or (number_6 is not null and trycnt_6 >= %lld)"
      " or (number_7 is not null and trycnt_7 >= %lld)"
      " or (number_8 is not null and trycnt_8 >= %lld)"
      " or (res_dial == %d)"
      ")"
      " and status = %d"
      " and in_use = %d"
      ";",

      json_string_value(json_object_get(j_dlma, "dl_table")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_1")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_2")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_3")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_4")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_5")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_6")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_7")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_8")),
      AST_CONTROL_ANSWER,
      E_DL_STATUS_IDLE,
      E_USE_OK
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get finished dial list count info.");
    return -1;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_res == NULL) {
    return -1;
  }

  ret = json_integer_value(json_object_get(j_res, "count(*)"));
  return ret;
}

/**
 * Get available list count.
 * \param j_dlma
 * \param j_plan
 * \return
 */
int get_ob_dl_list_cnt_available(json_t* j_dlma, json_t* j_plan)
{
  char* sql;
  int ret;
  json_t* j_res;

  asprintf(&sql, "select count(*)"
      " from `%s` where "
      "("
      " (number_1 is not null and trycnt_1 < %lld)"
      " or (number_2 is not null and trycnt_2 < %lld)"
      " or (number_3 is not null and trycnt_3 < %lld)"
      " or (number_4 is not null and trycnt_4 < %lld)"
      " or (number_5 is not null and trycnt_5 < %lld)"
      " or (number_6 is not null and trycnt_6 < %lld)"
      " or (number_7 is not null and trycnt_7 < %lld)"
      " or (number_8 is not null and trycnt_8 < %lld)"
      ")"
      " and res_dial != %d"
      " and status = %d"
      " and in_use = %d"
      ";",
      json_string_value(json_object_get(j_dlma, "dl_table")),

      json_integer_value(json_object_get(j_plan, "max_retry_cnt_1")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_2")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_3")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_4")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_5")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_6")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_7")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_8")),

      AST_CONTROL_ANSWER,
      E_DL_STATUS_IDLE,
      E_USE_OK
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get finished dial list count info.");
    return -1;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_res == NULL) {
    return -1;
  }

  ret = json_integer_value(json_object_get(j_res, "count(*)"));
  return ret;
}

/**
 * Get dl list tryed count.
 * \param j_dlma
 * \param j_plan
 * \return
 */
int get_ob_dl_list_cnt_dialing(json_t* j_dlma)
{
  char* sql;
  int ret;
  json_t* j_res;

  if(j_dlma == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return -1;
  }

  asprintf(&sql, "select count(*) from '%s' where status != %d and in_use = %d;",
      json_string_value(json_object_get(j_dlma, "dl_table"))? : "",
      E_DL_STATUS_IDLE,
      E_USE_OK
      );
  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);

  ret = json_integer_value(json_object_get(j_res, "count(*)"));
  json_decref(j_res);

  return ret;
}

/**
 * Get dialing list count.
 * \param j_dlma
 * \param j_plan
 * \return
 */
int get_ob_dl_list_cnt_tried(json_t* j_dlma)
{
  char* sql;
  int ret;
  json_t* j_res;

  asprintf(&sql, "select "
      " sum(trycnt_1 + trycnt_2 + trycnt_3 + trycnt_4 + trycnt_5 + trycnt_6 + trycnt_7 + trycnt_8) as trycnt"
      " from `%s` where in_use = %d"
      ";",
      json_string_value(json_object_get(j_dlma, "dl_table")),
      E_USE_OK
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get dial list info.");
    return -1;
  }

  j_res = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_res == NULL) {
    return -1;
  }

  ret = json_integer_value(json_object_get(j_res, "trycnt"));
  json_decref(j_res);

  return ret;
}


/**
 * Create dialing json object
 * @param j_camp
 * @param j_plan
 * @param j_dlma
 * @param j_dl_list
 * @return
 */
json_t* create_dial_info(
    json_t* j_plan,
    json_t* j_dl_list,
    json_t* j_dest
    )
{
  json_t* j_dial;
  json_t* j_dial_dest;
  json_t* j_dial_dl;
  json_t* j_dial_plan;
  json_t* j_variables;
  json_t* j_tmp;
  const char* tmp_const;
  char* tmp;

  if((j_plan == NULL) || (j_dl_list == NULL) || (j_dest == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get dial destination
  j_dial_dest = create_ob_dial_destination_info(j_dest);
  if(j_dial_dest == NULL) {
    slog(LOG_ERR, "Could not create correct dial destination.");
    return NULL;
  }

  // get dial dl
  j_dial_dl = create_dial_dl_info(j_dl_list, j_plan);
  if(j_dial_dl == NULL) {
    slog(LOG_ERR, "Could not create correct dial dl info.");
    json_decref(j_dial_dest);
    return NULL;
  }

  j_dial_plan = create_ob_dial_plan_info(j_plan);
  if(j_dial_plan == NULL) {
    slog(LOG_ERR, "Could not create correct dial plan info.");
    json_decref(j_dial_dest);
    json_decref(j_dial_dl);
    return NULL;
  }

  // create dial
  j_dial = json_object();
  json_object_update(j_dial, j_dial_dest);
  json_object_update(j_dial, j_dial_dl);
  json_object_update(j_dial, j_dial_plan);
  json_decref(j_dial_dest);
  json_decref(j_dial_dl);
  json_decref(j_dial_plan);

  // update variables
  j_variables = json_object();

  tmp_const = json_string_value(json_object_get(j_dial, "plan_variables"));
  if((tmp_const != NULL) || (strlen(tmp_const) != 0)) {
    j_tmp = json_loads(tmp_const, JSON_DECODE_ANY, NULL);
    if(j_tmp != NULL) {
      json_object_update(j_variables, j_tmp);
    }
    json_decref(j_tmp);
  }

  tmp_const = json_string_value(json_object_get(j_dial, "dest_variables"));
  if((tmp_const != NULL) || (strlen(tmp_const) != 0)) {
    j_tmp = json_loads(tmp_const, JSON_DECODE_ANY, NULL);
    if(j_tmp != NULL) {
      json_object_update(j_variables, j_tmp);
    }
    json_decref(j_tmp);
  }

  tmp_const = json_string_value(json_object_get(j_dial, "dl_variables"));
  if((tmp_const != NULL) || (strlen(tmp_const) != 0)) {
    j_tmp = json_loads(tmp_const, JSON_DECODE_ANY, NULL);
    if(j_tmp != NULL) {
      json_object_update(j_variables, j_tmp);
    }
    json_decref(j_tmp);
  }

  tmp = json_dumps(j_variables, JSON_ENCODE_ANY);
  json_object_set_new(j_dial, "variables", json_string(tmp?:""));
  sfree(tmp);
  json_decref(j_variables);

  return j_dial;
}

/**
 * Create dl_list's dial address.
 * @param j_camp
 * @param j_plan
 * @param j_dl_list
 * @return
 */
static char* create_chan_addr_for_dial(json_t* j_plan, json_t* j_dl_list, int dial_num_point)
{
  char* dest_addr;
  char* chan_addr;
  const char* trunk_name;
  const char* tech_name;

  if(dial_num_point < 0) {
    slog(LOG_WARNING, "Wrong dial number point.");
    return NULL;
  }

  trunk_name = json_string_value(json_object_get(j_plan, "trunk_name"));
  if(trunk_name == NULL) {
    trunk_name = "";
  }

  tech_name = json_string_value(json_object_get(j_plan, "tech_name"));
  if(tech_name == NULL) {
    tech_name = "";
  }

  // get dial number
  dest_addr = get_dial_number(j_dl_list, dial_num_point);
  if(dest_addr == NULL) {
    slog(LOG_WARNING, "Could not get destination address.");
    return NULL;
  }

  // create dial addr
  if(strlen(trunk_name) > 0) {
    asprintf(&chan_addr, "%s/%s@%s", tech_name, dest_addr, trunk_name);
  }
  else {
    asprintf(&chan_addr, "%s/%s", tech_name, dest_addr);
  }
  slog(LOG_DEBUG, "Created dialing channel address. chan_addr[%s].", chan_addr);
  sfree(dest_addr);

  return chan_addr;
}

/**
 * Return dial number of j_dlist.
 * @param j_dlist
 * @param cnt
 * @return
 */
static char* get_dial_number(json_t* j_dlist, const int cnt)
{
  char* res;
  char* tmp;

  asprintf(&tmp, "number_%d", cnt);

  asprintf(&res, "%s", json_string_value(json_object_get(j_dlist, tmp)));
  sfree(tmp);

  return res;
}

/**
 * Create dl_result
 * @param dialing
 * @return
 */
json_t* create_json_for_dl_result(json_t* j_dialing)
{
  json_t* j_res;
  const char* tmp_const;

  if(j_dialing == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_res = json_deep_copy(j_dialing);

  slog(LOG_DEBUG, "Check value. dialing_uuid[%s], camp_uuid[%s], plan_uuid[%s], dlma_uuid[%s], dl_list_uuid[%s]",
      json_string_value(json_object_get(j_res, "uuid")),
      json_string_value(json_object_get(j_res, "uuid_camp")),
      json_string_value(json_object_get(j_res, "uuid_plan")),
      json_string_value(json_object_get(j_res, "uuid_dlma")),
      json_string_value(json_object_get(j_res, "uuid_dl_list"))
      );

  // result_info_enable
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "result_info_enable"));
  if((tmp_const != NULL) && (atoi(tmp_const) == 1)) {
    // write info
    // already copied it.
  }
  else {
    // otherwise, remove
    json_object_del(j_res, "info_camp");
    json_object_del(j_res, "info_dial");
    json_object_del(j_res, "info_plan");
    json_object_del(j_res, "info_dest");
    json_object_del(j_res, "info_dlma");
    json_object_del(j_res, "info_dl_list");
    json_object_del(j_res, "info_events");
  }

  return j_res;
}

/**
 * Validate given dial list.
 * @param j_dl
 * @return
 */
bool validate_ob_dl(json_t* j_data)
{
  int ret;
  const char* tmp_const;
  json_t* j_tmp;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // variables
  j_tmp = json_object_get(j_data, "variables");
  if(j_tmp != NULL) {
    if(json_is_object(j_tmp) != true) {
      slog(LOG_NOTICE, "Wrong input type for variable. It should be json_object type.");
      return false;
    }
  }

  // dlma_uuid
  tmp_const = json_string_value(json_object_get(j_data, "dlma_uuid"));
  if(tmp_const == NULL) {
    slog(LOG_DEBUG, "The dlma_uuid is set.");
    return false;
  }
  ret = is_exist_ob_dlma(tmp_const);
  if(ret == false) {
    slog(LOG_DEBUG, "The dlma_uuid is not valid.");
    return false;
  }

  return true;
}

/**
 * Create dl_list
 * @param j_dl
 * @return
 */
json_t* create_ob_dl(json_t* j_dl)
{
  int ret;
  char* uuid;
  char* tmp;
  json_t* j_tmp;

  if(j_dl == NULL) {
    slog(LOG_DEBUG, "Fired create_ob_dl.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired create_ob_dl.");

  // create default and update
  j_tmp = create_ob_dl_default();
  json_object_update_existing(j_tmp, j_dl);

  // uuid
  uuid = gen_uuid();
  json_object_set_new(j_tmp, "uuid", json_string(uuid));

  // create timestamp
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_create", json_string(tmp));
  sfree(tmp);

  slog(LOG_NOTICE, "Create dl_list. dl_uuid[%s], dlma_uuid[%s], name[%s]",
      json_string_value(json_object_get(j_tmp, "uuid")),
      json_string_value(json_object_get(j_tmp, "dlma_uuid")),
      json_string_value(json_object_get(j_tmp, "name"))
      );

  ret = db_ctx_insert(g_db_ob, "ob_dl_list", j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    sfree(uuid);
    return NULL;
  }

  slog(LOG_DEBUG, "Getting created ob_dl info. uuid[%s]", uuid);
  j_tmp = get_ob_dl(uuid);
  sfree(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get created ob_dl info.");
    return NULL;
  }

  return j_tmp;
}

/**
 * delete dl_list
 * @param uuid
 * @return
 */
bool delete_dl_list(const char* uuid)
{
  json_t* j_tmp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    return false;
  }

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_USE_NO));
  sfree(tmp);

  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_dl_list set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete ob_dl_list. uuid[%s]", uuid);
    return false;
  }

//  // send notification
//  send_manager_evt_out_dl_list_delete(uuid);

  return true;
}

/**
 *
 * @param j_dl_list
 * @param j_plan
 * @return
 */
static json_t* create_dial_dl_info(json_t* j_dl_list, json_t* j_plan)
{
  int index;
  int count;
  char* addr;
  char* channel;
  json_t* j_res;
  char* channel_id;
  char* other_channel_id;

  if((j_dl_list == NULL) || (j_plan == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  char* tmp;
  tmp = json_dumps(j_dl_list, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Detail dl_list info. tmp[%s]", tmp);
  sfree(tmp);

  tmp = json_dumps(j_plan, JSON_ENCODE_ANY);
  slog(LOG_DEBUG, "Detail plan info. tmp[%s]", tmp);
  sfree(tmp);

  // get dial number point(index)
  index = get_dial_num_point(j_dl_list, j_plan);
  if(index < 0) {
    slog(LOG_ERR, "Could not find correct number count.");
    return NULL;
  }

  // get dial count
  count = get_dial_try_cnt(j_dl_list, index);
  if(count == -1) {
    slog(LOG_ERR, "Could not get correct dial count number.");
    return NULL;
  }

  // get dial address
  addr = get_dial_number(j_dl_list, index);
  if(addr == NULL) {
    slog(LOG_ERR, "Could not get correct dial address.");
    return NULL;
  }

  // create destination channel address.
  channel = create_chan_addr_for_dial(j_plan, j_dl_list, index);
  if(channel == NULL) {
    slog(LOG_ERR, "Could not get correct channel address.");
    sfree(addr);
    return NULL;
  }

  channel_id = gen_uuid();
  other_channel_id = gen_uuid();

  j_res = json_pack(
      "{"
      "s:s, "
      "s:s, s:s, s:i, s:i, s:s, "
      "s:s, s:s"
      "}",

      "uuid",    json_string_value(json_object_get(j_dl_list, "uuid")),

      "dial_channel",   channel,
      "dial_addr",      addr,
      "dial_index",      index,
      "dial_trycnt",    count,
      "dl_variables",    json_string_value(json_object_get(j_dl_list, "variables"))? : "",

      "channelid",      channel_id,
      "otherchannelid",  other_channel_id
      );
  sfree(channel);
  sfree(addr);
  sfree(channel_id);
  sfree(other_channel_id);

  return j_res;
}

/**
 * Return is this endable dl list.
 * \param j_dlma
 * \param j_plan
 * \return
 */
bool is_endable_dl_list(json_t* j_dlma, json_t* j_plan)
{
  int ret;

  // check is there dial-able dl list.
  ret = check_more_dl_list(j_dlma, j_plan);
  if(ret == true) {
    return false;
  }

  return true;
}

/**
 * Check is the given dl is older than given plan's retry delay time.
 * @param j_dlma
 * @param j_dl
 * @param j_plan
 * @return
 */
static bool is_over_retry_delay(json_t* j_dlma, json_t* j_dl, json_t* j_plan)
{
  json_t* j_tmp;
  const char* tm_last_hangup;
  int retry_delay;
  char* sql;
  int ret;

  if((j_dlma == NULL) || (j_dl == NULL) || (j_plan == NULL)) {
    slog(LOG_WARNING, "Wrong input parameters.");
    return false;
  }

  // check last hangup timestamp.
  // if there's no tm_last_hangup, then it's first try.
  tm_last_hangup = json_string_value(json_object_get(j_dl, "tm_last_hangup"));
  if((tm_last_hangup == NULL) || (strlen(tm_last_hangup) == 0)) {
    return true;
  }

  retry_delay = json_integer_value(json_object_get(j_plan, "retry_delay"));

  asprintf(&sql, "select uuid from '%s' where uuid = '%s' and ((strftime('%%s', 'now') - strftime('%%s', tm_last_hangup)) > %d);",
      json_string_value(json_object_get(j_dlma, "dl_table")),
      json_string_value(json_object_get(j_dl, "uuid")),
      retry_delay
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);

  if(ret == false) {
    return false;
  }

  j_tmp = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

static json_t* get_dls_uuid_available(json_t* j_dlma, json_t* j_plan, int count)
{
  char* sql;
  const char* uuid;
  int ret;
  json_t* j_res;
  json_t* j_tmp;

  asprintf(&sql, "select uuid, "
      "(trycnt_1 + trycnt_2 + trycnt_3 + trycnt_4 + trycnt_5 + trycnt_6 + trycnt_7 + trycnt_8) as trycnt"
      " from `%s` where ("
      "(number_1 is not null and trycnt_1 < %lld)"
      " or (number_2 is not null and trycnt_2 < %lld)"
      " or (number_3 is not null and trycnt_3 < %lld)"
      " or (number_4 is not null and trycnt_4 < %lld)"
      " or (number_5 is not null and trycnt_5 < %lld)"
      " or (number_6 is not null and trycnt_6 < %lld)"
      " or (number_7 is not null and trycnt_7 < %lld)"
      " or (number_8 is not null and trycnt_8 < %lld)"
      ")"
      " and res_dial != %d"
      " and status = %d"
      " order by trycnt asc"
      " limit %d"
      ";",
      json_string_value(json_object_get(j_dlma, "dl_table")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_1")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_2")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_3")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_4")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_5")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_6")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_7")),
      json_integer_value(json_object_get(j_plan, "max_retry_cnt_8")),
      AST_CONTROL_ANSWER,
      E_DL_STATUS_IDLE,
      count
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get dial list info.");
    return NULL;
  }

  j_res = json_array();
  while(1) {
    j_tmp = db_ctx_get_record(g_db_ob);
    if(j_tmp == NULL) {
      break;
    }

    uuid = json_string_value(json_object_get(j_tmp, "uuid"));
    if(uuid == NULL) {
      json_decref(j_tmp);
      continue;
    }

    json_array_append_new(j_res, json_string(uuid));
    json_decref(j_tmp);
  }
  db_ctx_free(g_db_ob);

  return j_res;
}


/**
 * Get available dl_list from database.
 * @param j_dlma
 * @param j_plan
 * @return
 */
static json_t* get_ob_dl_available(json_t* j_dlma, json_t* j_plan)
{
  json_t* j_uuids;
  json_t* j_res;
  json_t* j_tmp;
  const char* uuid;

  if((j_dlma == NULL) || (j_plan == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  j_uuids = get_dls_uuid_available(j_dlma, j_plan, 1);
  if(j_uuids == NULL) {
    return NULL;
  }

  j_tmp = json_array_get(j_uuids, 0);
  if(j_tmp == NULL) {
    json_decref(j_uuids);
    return NULL;
  }

  uuid = json_string_value(j_tmp);
  if(uuid == NULL) {
    json_decref(j_uuids);
    return NULL;
  }

  j_res = get_ob_dl(uuid);
  json_decref(j_uuids);

  return j_res;
}

///**
// * Increase given uuid dial list's try count.
// * @param uuid
// * @param idx
// * @return
// */
//bool increase_ob_dl_trycnt(const char* uuid, int idx)
//{
//  int ret;
//  int try_count;
//  char* try_count_field;
//  json_t* j_dl;
//  json_t* j_tmp;
//
//  if(uuid == NULL) {
//    slog(LOG_WARNING, "Wrong input parameter.");
//    return false;
//  }
//  slog(LOG_DEBUG, "Fired increase_ob_dl_trycnt. uuid[%s], idx[%d]", uuid, idx);
//
//  // check exist
//  ret = is_exist_ob_dl(uuid);
//  if(ret == false) {
//    slog(LOG_NOTICE, "Could not find correct ob_dl info. uuid[%s]", uuid);
//    return false;
//  }
//
//  // get ob_dl info
//  j_dl = get_ob_dl(uuid);
//  if(j_dl == NULL) {
//    slog(LOG_ERR, "Could not get correct ob_dl info. uuid[%s]", uuid);
//    return false;
//  }
//
//  // create string
//  asprintf(&try_count_field, "trycnt_%d", idx);
//
//  // get current try cnt
//  try_count = json_integer_value(json_object_get(j_dl, try_count_field));
//
//  // increase try_count
//  try_count++;
//
//  // update
//  json_object_set(j_dl, try_count_field, json_integer(try_count));
//  sfree(try_count_field);
//
//  j_tmp = update_ob_dl(j_dl);
//  json_decref(j_dl);
//  json_decref(j_tmp);
//
//  return true;
//}

bool update_ob_dl_after_originate(json_t* j_dialing)
{
  char* timestamp;
  const char* tmp_const;
  char* try_count_field;
  json_t* j_dl_update;
  json_t* j_tmp;

  if(j_dialing == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get info_dial
  tmp_const = json_string_value(json_object_get(j_dialing, "info_dial"));
  if(tmp_const == NULL) {
    slog(LOG_ERR, "Could not get info_dial.");
    return false;
  }

  // get timestamp
  timestamp = get_utc_timestamp();

  // create trycnt string
  asprintf(&try_count_field, "trycnt_%lld", json_integer_value(json_object_get(j_dialing, "dial_index")));

  // create update dl_list
  j_dl_update = json_pack("{s:s, s:i, s:i, s:s, s:s, s:s, s:s}",
      "uuid",               json_string_value(json_object_get(j_dialing, "uuid_dl_list"))? : "",
      try_count_field,      json_integer_value(json_object_get(j_dialing, "dial_trycnt")),
      "status",             E_DL_STATUS_DIALING,
      "dialing_uuid",       json_string_value(json_object_get(j_dialing, "uuid"))? : "",
      "dialing_camp_uuid",  json_string_value(json_object_get(j_dialing, "uuid_camp"))? : "",
      "dialing_plan_uuid",  json_string_value(json_object_get(j_dialing, "uuid_plan"))? : "",
      "tm_last_dial",       timestamp? : ""
      );
  sfree(timestamp);
  sfree(try_count_field);
  if(j_dl_update == NULL) {
    slog(LOG_ERR, "Could not create updated dl info.");
    return false;
  }

  // dl update
  j_tmp = update_ob_dl(j_dl_update);
  json_decref(j_dl_update);
  if(j_tmp == NULL) {
    clear_dl_list_dialing(json_string_value(json_object_get(j_dialing, "uuid_dl_list")));
    slog(LOG_ERR, "Could not update dial list info.");
    return false;
  }
  json_decref(j_tmp);

  return true;
}

bool update_dl_list_after_create_dialing_info_(rb_dialing* dialing)
{
  char* tmp;
  char* try_count_field;
  json_t* j_dl_update;
  json_t* j_tmp;

  // get timestamp
  tmp = get_utc_timestamp();

  // get
  asprintf(&try_count_field, "trycnt_%lld",
      json_integer_value(json_object_get(dialing->j_dialing, "dial_index"))
      );

  // create update dl_list
  j_dl_update = json_pack("{s:s, s:I, s:i, s:s, s:s, s:s, s:s}",
      "uuid",                 json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")),
      try_count_field,      json_integer_value(json_object_get(dialing->j_dialing, "dial_trycnt")),
      "status",               E_DL_STATUS_DIALING,
      "dialing_uuid",         dialing->uuid,
      "dialing_camp_uuid",  json_string_value(json_object_get(dialing->j_dialing, "camp_uuid")),
      "dialing_plan_uuid",  json_string_value(json_object_get(dialing->j_dialing, "plan_uuid")),
      "tm_last_dial",         tmp
      );
  sfree(tmp);
  sfree(try_count_field);

  // dl update
  j_tmp = update_ob_dl(j_dl_update);
  json_decref(j_dl_update);
  if(j_tmp == NULL) {
    rb_dialing_destory(dialing);
    clear_dl_list_dialing(json_string_value(json_object_get(dialing->j_dialing, "dl_list_uuid")));
    slog(LOG_ERR, "Could not update dial list info.");
    return false;
  }
  json_decref(j_tmp);

  return true;
}

/**
 * Return existence of given ob_dlma uuid.
 * @param uuid
 * @return
 */
bool is_exist_ob_dlma(const char* uuid)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  asprintf(&sql, "select count(*) from ob_dl_list_ma where uuid=\"%s\" and in_use=%d;", uuid, E_USE_OK);

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_dlma info. uuid[%s]", uuid);
    return false;
  }

  j_tmp = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dlma info. uuid[%s]", uuid);
    return false;
  }

  // result
  ret = json_integer_value(json_object_get(j_tmp, "count(*)"));
  json_decref(j_tmp);
  if(ret <= 0) {
    return false;
  }

  return true;
}

/**
 * Return existence of given ob_dl uuid.
 * @param uuid
 * @return
 */
bool is_exist_ob_dl(const char* uuid)
{
  char* sql;
  int ret;
  json_t* j_tmp;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired is_exist_ob_dl.");

  asprintf(&sql, "select count(*) from ob_dl_list where uuid=\"%s\" and in_use=%d;",
      uuid,
      E_USE_OK
      );

  ret = db_ctx_query(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_ERR, "Could not get ob_dl info. uuid[%s]", uuid);
    return false;
  }

  j_tmp = db_ctx_get_record(g_db_ob);
  db_ctx_free(g_db_ob);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get correct ob_dl info. uuid[%s]", uuid);
    return false;
  }

  // result
  ret = json_integer_value(json_object_get(j_tmp, "count(*)"));
  json_decref(j_tmp);
  if(ret <= 0) {
    return false;
  }

  return true;
}

/**
 * Delete ob_dls related with given dlma_uuid
 * @param dlma_uuid
 * @return
 */
bool delete_ob_dls_by_dlma_uuid(const char* dlma_uuid)
{
  char* sql;
  char* timestamp;
  char* tmp;
  json_t* j_tmp;
  int ret;

  if(dlma_uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired delete_ob_dls_by_dlma_uuid. dlma_uuid[%s]", dlma_uuid);

  j_tmp = json_object();
  timestamp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(timestamp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_USE_NO));
  sfree(timestamp);

  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_dl_list set %s where dlma_uuid=\"%s\";", tmp, dlma_uuid);
  sfree(tmp);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    return false;
  }

  return true;
}

/**
 * Delete ob_dl_list record.
 * @param uuid
 * @return
 */
json_t* delete_ob_dl(const char* uuid)
{
  json_t* j_tmp;
  char* tmp;
  char* sql;
  int ret;

  if(uuid == NULL) {
    // invalid parameter.
    slog(LOG_ERR, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired delete_ob_dl. uuid[%s]", uuid);

  j_tmp = json_object();
  tmp = get_utc_timestamp();
  json_object_set_new(j_tmp, "tm_delete", json_string(tmp));
  json_object_set_new(j_tmp, "in_use", json_integer(E_USE_NO));
  sfree(tmp);

  tmp = db_ctx_get_update_str(j_tmp);
  json_decref(j_tmp);
  asprintf(&sql, "update ob_dl_list set %s where uuid=\"%s\";", tmp, uuid);
  sfree(tmp);

  ret = db_ctx_exec(g_db_ob, sql);
  sfree(sql);
  if(ret == false) {
    slog(LOG_WARNING, "Could not delete ob_dl_list. uuid[%s]", uuid);
    return NULL;
  }

  j_tmp = get_deleted_ob_dl(uuid);
  if(j_tmp == NULL) {
    slog(LOG_ERR, "Could not get deleted ob_dl_list info. uuid[%s]", uuid);
    return NULL;
  }

  return j_tmp;
}

/**
 * Get deleted dial list.
 * @return
 */
static json_t* get_deleted_ob_dl(const char* uuid)
{
  json_t* j_res;

  if(uuid == NULL) {
      slog(LOG_WARNING, "Wrong input parameter.");
      return NULL;
    }
  slog(LOG_DEBUG, "Fired get_deleted_dl. uuid[%s]", uuid);

  j_res = get_ob_dl_use(uuid, E_USE_NO);
  if(j_res == NULL) {
    slog(LOG_WARNING, "Could not get deleted ob_dl_list info.");
    return NULL;
  }

  return j_res;
}

bool update_ob_dl_hangup(
    const char* uuid,
    int res_dial,
    const char* res_dial_detail,
    int res_hangup,
    const char* res_hangup_detail
    )
{
  char* timestamp;
  json_t* j_update;
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ob_dl_hangup. uuid[%s], res_dial[%d], res_dial_detail[%s], res_hangup[%d], res_hangup_detail[%s]",
      uuid, res_dial, res_dial_detail? : "", res_hangup, res_hangup_detail? : ""
      );

  // create dl_list for update
  timestamp = get_utc_timestamp();
  j_update = json_pack("{"
      "s:s, s:i, "
      "s:o, s:o, s:o, "
      "s:i, s:s, s:i, s:s, "
      "s:s"
      "}",
      "uuid",               uuid,
      "status",             E_DL_STATUS_IDLE,

      "dialing_uuid",       json_null(),
      "dialing_camp_uuid",  json_null(),
      "dialing_plan_uuid",  json_null(),

      "res_dial",           res_dial,
      "res_dial_detail",    res_dial_detail? : "",
      "res_hangup",         res_hangup,
      "res_hangup_detail",  res_hangup_detail? : "",

      "tm_last_hangup",     timestamp
      );
  sfree(timestamp);
  if(j_update == NULL) {
    slog(LOG_ERR, "Could not create update dl_list.");
    return false;
  }

  j_res = update_ob_dl(j_update);
  json_decref(j_update);
  json_decref(j_res);

  return true;
}

/**
 * Update ob_dl's status info
 * @param uuid
 * @param status
 * @return
 */
bool update_ob_dl_status(const char* uuid, E_DL_STATUS_T status)
{
  json_t* j_update;
  json_t* j_res;

  if(uuid == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired update_ob_dl_status. uuid[%s], status[%d]", uuid, status);

  // create update info
  j_update = json_pack("{s:s, s:i}",
      "uuid",   uuid,
      "status", status
      );
  if(j_update == NULL) {
    slog(LOG_ERR, "Could not create update dl_list info.");
    return false;
  }

  // update dial list
  j_res = update_ob_dl(j_update);
  json_decref(j_update);
  json_decref(j_res);

  return true;
}

static json_t* create_ob_dl_default(void)
{
  json_t* j_res;

  j_res = json_pack("{"
      "s:o,"
      "s:o, s:o, s:i,"
      "s:o,"
      "s:o,"
      "s:o, s:o, s:o, s:o, s:o, s:o, s:o, s:o, s:o, "
      "s:o"
      "}",

      "dlma_uuid",    json_null(),

      "name",     json_null(),
      "detail",   json_null(),
      "status",   DEF_DL_STATUS,

      "resv_target",  json_null(),

      "ukey", json_null(),

      "email",    json_null(),
      "number_1", json_null(),
      "number_2", json_null(),
      "number_3", json_null(),
      "number_4", json_null(),
      "number_5", json_null(),
      "number_6", json_null(),
      "number_7", json_null(),
      "number_8", json_null(),

      "variables",  json_object()
      );

  return j_res;
}

