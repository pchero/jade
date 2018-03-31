/*
 * dl_handler.h
 *
 *  Created on: Nov 29, 2015
 *    Author: pchero
 */

#ifndef SRC_DL_HANDLER_H_
#define SRC_DL_HANDLER_H_

#include <stdbool.h>
#include <jansson.h>

#include "ob_dialing_handler.h"

/**
 * dl_list.status
 */
typedef enum _E_DL_STATUS_T
{
  E_DL_STATUS_IDLE     = 0,      //!< idle
  E_DL_STATUS_DIALING  = 1,    //!< dialing
  E_DL_STATUS_RESERVED   = 2,  //!< reserved for preview dialing
//  E_DL_QUEUEING   = 2,  ///< not in used.
} E_DL_STATUS_T;

int ob_get_current_dialing_dl_cnt(const char* camp_uuid, const char* dl_table);
int ob_get_dial_num_point(json_t* j_dl_list, json_t* j_plan);
int ob_get_dial_try_cnt(json_t* j_dl_list, int dial_num_point);

json_t* ob_get_dl(const char* uuid);
json_t* ob_get_dls_uuid_by_dlma_count(const char* dlma_uuid, int count);
json_t* ob_get_dls_by_dlma_count(const char* dlma_uuid, int count);
json_t* ob_get_dls_by_count(int count);
json_t* ob_get_dls_by_status(E_DL_STATUS_T status);
json_t* ob_get_dls_error(void);

json_t* ob_create_dl(json_t* j_dl);
json_t* ob_update_dl(json_t* j_dl);
json_t* ob_delete_dl(const char* uuid);
bool ob_delete_dls_by_dlma_uuid(const char* dlma_uuid);
bool ob_delete_dl_list(const char* uuid);
bool ob_validate_dl(json_t* j_dl);

int ob_get_dl_count_by_dlma_uuid(const char* dlma_uuid);
int ob_get_dl_list_cnt_total(json_t* j_dlma);
int ob_get_dl_list_cnt_finshed(json_t* j_dlma, json_t* j_plan);
int ob_get_dl_list_cnt_available(json_t* j_dlma, json_t* j_plan);
int ob_get_dl_list_cnt_dialing(json_t* j_dlma);
int ob_get_dl_list_cnt_tried(json_t* j_dlma);

json_t* ob_get_dl_available_for_dial(json_t* j_dlma, json_t* j_plan);
bool ob_is_endable_dl_list(json_t* j_dlma, json_t* j_plan);
void ob_clear_dl_list_dialing(const char* uuid);

json_t* ob_create_dial_info(json_t* j_plan, json_t* j_dl_list, json_t* j_dest);
json_t* ob_create_json_for_dl_result(json_t* j_dialing);

bool ob_update_dl_after_originate(json_t* dialing);
bool ob_update_dl_status(const char* uuid, E_DL_STATUS_T status);
bool ob_update_dl_hangup(
    const char* uuid,
    int res_dial,
    const char* res_dial_detail,
    int res_hangup,
    const char* res_hangup_detail
    );

bool ob_is_dlma_exist(const char* uuid);
bool ob_is_dl_exsit(const char* uuid);

#endif /* SRC_DL_HANDLER_H_ */
