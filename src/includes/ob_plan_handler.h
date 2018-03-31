/*
 * plan_handler.h
 *
 *  Created on: Nov 30, 2015
 *    Author: pchero
 */

#ifndef SRC_PLAN_HANDLER_H_
#define SRC_PLAN_HANDLER_H_

#include <jansson.h>

typedef enum _E_DIAL_MODE {
  E_DIAL_MODE_NONE = 0,       /// No dial mode
  E_DIAL_MODE_PREDICTIVE = 1,   ///
  E_DIAL_MODE_PREVIEW = 2,    ///
  E_DIAL_MODE_POWER = 3,      ///
  E_DIAL_MODE_ROBO = 4,       ///
  E_DIAL_MODE_REDIRECT = 5,     ///
} E_DIAL_MODE;

/**
 * Determine when the dial list end.
 */
typedef enum _E_PLAN_DL_END_HANDLE {
  E_PLAN_DL_END_NONSTOP    = 0,  /// Keep current status.
  E_PLAN_DL_END_STOP       = 1,  /// Stop the campaign.
} E_PLAN_DL_END_HANDLE;


bool ob_init_plan(void);

bool ob_validate_plan(json_t* j_data);
json_t* ob_create_plan(json_t* j_plan);
json_t* ob_delete_plan(const char* uuid);
json_t* ob_update_plan(const json_t* j_plan);
json_t* ob_get_plan(const char* uuid);
json_t* ob_get_plans_all(void);
json_t* ob_get_plans_all_uuid(void);

json_t* ob_create_dial_plan_info(json_t* j_plan);
bool ob_is_plan_enable(json_t* j_plan);
bool ob_is_dl_handle_nonstop(json_t* j_plan);
bool ob_is_plan_exist(const char* uuid);
bool ob_is_plan_deletable(const char* uuid);

#endif /* SRC_PLAN_HANDLER_H_ */
