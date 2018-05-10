/*
 * http_handler.c
 *
 *  Created on: Feb 2, 2017
 *      Author: pchero
 */

#define _GNU_SOURCE

#include <jansson.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <string.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "http_handler.h"
#include "resource_handler.h"
#include "base64.h"

#include "ob_http_handler.h"
#include "voicemail_handler.h"
#include "core_handler.h"
#include "agent_handler.h"
#include "sip_handler.h"
#include "pjsip_handler.h"
#include "queue_handler.h"
#include "park_handler.h"
#include "dialplan_handler.h"
#include "user_handler.h"
#include "me_handler.h"
#include "admin_handler.h"
#include "manager_handler.h"

#define API_VER "0.1"

#define DEF_REG_MSGNAME "msg[0-9]{4}"

#define DEF_USER_PERM_ADMIN    "admin"
#define DEF_USER_PERM_USER     "user"

static bool init_https(void);

static char* get_data_from_request(evhtp_request_t* req);

// ping
static void cb_htp_ping(evhtp_request_t *req, void *a);

// admin
static void cb_htp_admin_login(evhtp_request_t *req, void *data);

// agent
static void cb_htp_agent_agents(evhtp_request_t *req, void *data);
static void cb_htp_agent_agents_detail(evhtp_request_t *req, void *data);

// core
static void cb_htp_core_agis(evhtp_request_t *req, void *data);
static void cb_htp_core_agis_detail(evhtp_request_t *req, void *data);
static void cb_htp_core_channels(evhtp_request_t *req, void *data);
static void cb_htp_core_channels_detail(evhtp_request_t *req, void *data);
static void cb_htp_core_modules(evhtp_request_t *req, void *data);
static void cb_htp_core_modules_detail(evhtp_request_t *req, void *data);
static void cb_htp_core_systems(evhtp_request_t *req, void *data);
static void cb_htp_core_systems_detail(evhtp_request_t *req, void *data);

// dp
static void cb_htp_dp_dialplans(evhtp_request_t *req, void *data);
static void cb_htp_dp_dialplans_detail(evhtp_request_t *req, void *data);
static void cb_htp_dp_dpmas(evhtp_request_t *req, void *data);
static void cb_htp_dp_dpmas_detail(evhtp_request_t *req, void *data);
static void cb_htp_dp_config(evhtp_request_t *req, void *data);

// manager
static void cb_htp_manager_login(evhtp_request_t *req, void *data);
static void cb_htp_manager_info(evhtp_request_t *req, void *data);
static void cb_htp_manager_users(evhtp_request_t *req, void *data);
static void cb_htp_manager_users_detail(evhtp_request_t *req, void *data);
static void cb_htp_manager_trunks(evhtp_request_t *req, void *data);
static void cb_htp_manager_trunks_detail(evhtp_request_t *req, void *data);

// me
static void cb_htp_me_buddies(evhtp_request_t *req, void *data);
static void cb_htp_me_buddies_detail(evhtp_request_t *req, void *data);
static void cb_htp_me_calls(evhtp_request_t *req, void *data);
static void cb_htp_me_calls_detail(evhtp_request_t *req, void *data);
static void cb_htp_me_contacts(evhtp_request_t *req, void *data);
static void cb_htp_me_chats(evhtp_request_t *req, void *data);
static void cb_htp_me_chats_detail(evhtp_request_t *req, void *data);
static void cb_htp_me_chats_detail_messages(evhtp_request_t *req, void *data);
static void cb_htp_me_info(evhtp_request_t *req, void *data);
static void cb_htp_me_login(evhtp_request_t *req, void *data);
static void cb_htp_me_search(evhtp_request_t *req, void *data);

// park
static void cb_htp_park_config(evhtp_request_t *req, void *data);
static void cb_htp_park_configs(evhtp_request_t *req, void *data);
static void cb_htp_park_configs_detail(evhtp_request_t *req, void *data);
static void cb_htp_park_parkinglots(evhtp_request_t *req, void *data);
static void cb_htp_park_parkinglots_detail(evhtp_request_t *req, void *data);
static void cb_htp_park_parkedcalls(evhtp_request_t *req, void *data);
static void cb_htp_park_parkedcalls_detail(evhtp_request_t *req, void *data);
static void cb_htp_park_settings(evhtp_request_t *req, void *data);
static void cb_htp_park_settings_detail(evhtp_request_t *req, void *data);

// pjsip
static void cb_htp_pjsip_aors(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_aors_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_auths(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_auths_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_contacts(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_contacts_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_endpoints(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_endpoints_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_identifies(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_identifies_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_registrations(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_registrations_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_transports(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_transports_detail(evhtp_request_t *req, void *data);

static void cb_htp_pjsip_config(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_configs(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_configs_detail(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_settings(evhtp_request_t *req, void *data);
static void cb_htp_pjsip_settings_detail(evhtp_request_t *req, void *data);

// queue
static void cb_htp_queue_entries(evhtp_request_t *req, void *data);
static void cb_htp_queue_entries_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_members(evhtp_request_t *req, void *data);
static void cb_htp_queue_members_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_queues(evhtp_request_t *req, void *data);
static void cb_htp_queue_queues_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_config(evhtp_request_t *req, void *data);
static void cb_htp_queue_configs(evhtp_request_t *req, void *data);
static void cb_htp_queue_configs_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_settings(evhtp_request_t *req, void *data);
static void cb_htp_queue_settings_detail(evhtp_request_t *req, void *data);
static void cb_htp_queue_statuses(evhtp_request_t *req, void *data);
static void cb_htp_queue_statuses_detail(evhtp_request_t *req, void *data);

// sip
static void cb_htp_sip_config(evhtp_request_t *req, void *data);
static void cb_htp_sip_configs(evhtp_request_t *req, void *data);
static void cb_htp_sip_configs_detail(evhtp_request_t *req, void *data);
static void cb_htp_sip_peers(evhtp_request_t *req, void *data);
static void cb_htp_sip_peers_detail(evhtp_request_t *req, void *data);
static void cb_htp_sip_registries(evhtp_request_t *req, void *data);
static void cb_htp_sip_registries_detail(evhtp_request_t *req, void *data);
static void cb_htp_sip_settings(evhtp_request_t *req, void *data);
static void cb_htp_sip_settings_detail(evhtp_request_t *req, void *data);

// user
static void cb_htp_user_login(evhtp_request_t *req, void *data);
static void cb_htp_user_contacts(evhtp_request_t *req, void *data);
static void cb_htp_user_contacts_detail(evhtp_request_t *req, void *data);
static void cb_htp_user_permissions(evhtp_request_t *req, void *data);
static void cb_htp_user_permissions_detail(evhtp_request_t *req, void *data);
static void cb_htp_user_users(evhtp_request_t *req, void *data);
static void cb_htp_user_users_detail(evhtp_request_t *req, void *data);

// voicemail/
static void cb_htp_voicemail_config(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_configs(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_configs_detail(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_users(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_users_detail(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_vms(evhtp_request_t *req, void *data);
static void cb_htp_voicemail_vms_msgname(evhtp_request_t *req, void *data);

// databases
static void cb_htp_databases(evhtp_request_t *req, void *data);
static void cb_htp_databases_key(evhtp_request_t *req, void *data);

// device_states
static void cb_htp_device_states(evhtp_request_t *req, void *data);
static void cb_htp_device_states_detail(evhtp_request_t *req, void *data);


extern app* g_app;
evhtp_t* g_htps = NULL;


bool http_init_handler(void)
{
  int ret;

  slog(LOG_DEBUG, "Fired init_http_handler.");

  // init https
  ret = init_https();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate https.");
    return false;
  }



  ///// apis v.1


  //// ^/admin/
  // login
  evhtp_set_regex_cb(g_htps, "^/v1/admin/login$", cb_htp_admin_login, NULL);



  //// ^/agent/
  // agents
  evhtp_set_regex_cb(g_htps, "^/v1/agent/agents/(.*)", cb_htp_agent_agents_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/agent/agents$", cb_htp_agent_agents, NULL);



  //// ^/core/
  // agis
  evhtp_set_regex_cb(g_htps, "^/v1/core/agis/(.*)", cb_htp_core_agis_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/core/agis$", cb_htp_core_agis, NULL);

  // channels
  evhtp_set_regex_cb(g_htps, "^/v1/core/channels/(.*)", cb_htp_core_channels_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/core/channels$", cb_htp_core_channels, NULL);

  // modules
  evhtp_set_regex_cb(g_htps, "^/v1/core/modules/(.*)", cb_htp_core_modules_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/core/modules$", cb_htp_core_modules, NULL);

  // systems
  evhtp_set_regex_cb(g_htps, "^/v1/core/systems/(.*)", cb_htp_core_systems_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/core/systems$", cb_htp_core_systems, NULL);



  //// ^/dp/
  // config
  evhtp_set_regex_cb(g_htps, "^/v1/dp/config$", cb_htp_dp_config, NULL);

  // dialplans
  evhtp_set_regex_cb(g_htps, "^/v1/dp/dialplans/(.*)", cb_htp_dp_dialplans_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/dp/dialplans$", cb_htp_dp_dialplans, NULL);
  // dpmas
  evhtp_set_regex_cb(g_htps, "^/v1/dp/dpmas/(.*)", cb_htp_dp_dpmas_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/dp/dpmas$", cb_htp_dp_dpmas, NULL);



  //// ^/manager/
  // login
  evhtp_set_regex_cb(g_htps, "^/v1/manager/login$", cb_htp_manager_login, NULL);

  // info
  evhtp_set_regex_cb(g_htps, "^/v1/manager/info$", cb_htp_manager_info, NULL);

  // trunks
  evhtp_set_regex_cb(g_htps, "^/v1/manager/trunks$", cb_htp_manager_trunks, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/manager/trunks/(.*)", cb_htp_manager_trunks_detail, NULL);

  // users
  evhtp_set_regex_cb(g_htps, "^/v1/manager/users$", cb_htp_manager_users, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/manager/users/("DEF_REG_UUID")$", cb_htp_manager_users_detail, NULL);


  //// ^/me/
  // buddies
  evhtp_set_regex_cb(g_htps, "^/v1/me/buddies$", cb_htp_me_buddies, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/me/buddies/("DEF_REG_UUID")$", cb_htp_me_buddies_detail, NULL);

  // calls
  evhtp_set_regex_cb(g_htps, "^/v1/me/calls$", cb_htp_me_calls, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/me/calls/("DEF_REG_UUID")$", cb_htp_me_calls_detail, NULL);

  // contacts
  evhtp_set_regex_cb(g_htps, "^/v1/me/contacts$", cb_htp_me_contacts, NULL);

  // chats
  evhtp_set_regex_cb(g_htps, "^/v1/me/chats$", cb_htp_me_chats, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/me/chats/("DEF_REG_UUID")$", cb_htp_me_chats_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/me/chats/("DEF_REG_UUID")/messages$", cb_htp_me_chats_detail_messages, NULL);

  // info
  evhtp_set_regex_cb(g_htps, "^/v1/me/info$", cb_htp_me_info, NULL);

  // login
  evhtp_set_regex_cb(g_htps, "^/v1/me/login$", cb_htp_me_login, NULL);

  // search
  evhtp_set_regex_cb(g_htps, "^/v1/me/search$", cb_htp_me_search, NULL);


  ////// ^/ob/
  ////// outbound modules
  // destinations
  evhtp_set_regex_cb(g_htps, "^/v1/ob/destinations$", ob_cb_htp_ob_destinations, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/ob/destinations/("DEF_REG_UUID")$", ob_cb_htp_ob_destinations_detail, NULL);

  // plans
  evhtp_set_regex_cb(g_htps, "^/v1/ob/plans$", ob_cb_htp_ob_plans, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/ob/plans/("DEF_REG_UUID")$", ob_cb_htp_ob_plans_detail, NULL);

  // campaigns
  evhtp_set_regex_cb(g_htps, "^/v1/ob/campaigns$", ob_cb_htp_ob_campaigns, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/ob/campaigns/("DEF_REG_UUID")$", ob_cb_htp_ob_campaigns_detail, NULL);

  // dlmas
  evhtp_set_regex_cb(g_htps, "^/v1/ob/dlmas$", ob_cb_htp_ob_dlmas, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/ob/dlmas/("DEF_REG_UUID")$", ob_cb_htp_ob_dlmas_detail, NULL);

  // dls
  evhtp_set_regex_cb(g_htps, "^/v1/ob/dls$", ob_cb_htp_ob_dls, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/ob/dls/("DEF_REG_UUID")$", ob_cb_htp_ob_dls_detail, NULL);

  // dialings
  evhtp_set_regex_cb(g_htps, "^/v1/ob/dialings$", ob_cb_htp_ob_dialings, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/ob/dialings/("DEF_REG_UUID")$", ob_cb_htp_ob_dialings_detail, NULL);



  //// ^/park/
  // config
  evhtp_set_regex_cb(g_htps, "^/v1/park/config$", cb_htp_park_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/v1/park/configs/(.*)", cb_htp_park_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/park/configs$", cb_htp_park_configs, NULL);

  // parkinglots
  evhtp_set_regex_cb(g_htps, "^/v1/park/parkinglots/(.*)", cb_htp_park_parkinglots_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/park/parkinglots$", cb_htp_park_parkinglots, NULL);

  // parkedcalls
  evhtp_set_regex_cb(g_htps, "^/v1/park/parkedcalls/(.*)", cb_htp_park_parkedcalls_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/park/parkedcalls$", cb_htp_park_parkedcalls, NULL);

  // settings
  evhtp_set_regex_cb(g_htps, "^/v1/park/settings/(.*)", cb_htp_park_settings_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/park/settings$", cb_htp_park_settings, NULL);


  //// ^/pjsip/
  // aors
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/aors/(.*)", cb_htp_pjsip_aors_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/aors$", cb_htp_pjsip_aors, NULL);

  // auths
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/auths/(.*)", cb_htp_pjsip_auths_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/auths$", cb_htp_pjsip_auths, NULL);

  // config
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/config$", cb_htp_pjsip_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/configs/(.*)", cb_htp_pjsip_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/configs$", cb_htp_pjsip_configs, NULL);

  // contacts
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/contacts/(.*)", cb_htp_pjsip_contacts_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/contacts$", cb_htp_pjsip_contacts, NULL);

  // endpoints
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/endpoints/(.*)", cb_htp_pjsip_endpoints_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/endpoints$", cb_htp_pjsip_endpoints, NULL);

  // identifies
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/identifies/(.*)", cb_htp_pjsip_identifies_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/identifies$", cb_htp_pjsip_identifies, NULL);

  // registrations
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/registrations/(.*)", cb_htp_pjsip_registrations_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/registrations$", cb_htp_pjsip_registrations, NULL);

  // transports
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/transports/(.*)", cb_htp_pjsip_transports_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/pjsip/transports$", cb_htp_pjsip_transports, NULL);



  //// ^/queue/
  // config
  evhtp_set_regex_cb(g_htps, "^/v1/queue/config$", cb_htp_queue_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/v1/queue/configs/(.*)", cb_htp_queue_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/queue/configs$", cb_htp_queue_configs, NULL);

  // entries
  evhtp_set_regex_cb(g_htps, "^/v1/queue/entries/(.*)", cb_htp_queue_entries_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/queue/entries$", cb_htp_queue_entries, NULL);

  // members
  evhtp_set_regex_cb(g_htps, "^/v1/queue/members/(.*)", cb_htp_queue_members_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/queue/members$", cb_htp_queue_members, NULL);

  // queues
  evhtp_set_regex_cb(g_htps, "^/v1/queue/queues/(.*)", cb_htp_queue_queues_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/queue/queues$", cb_htp_queue_queues, NULL);

  // settings
  evhtp_set_regex_cb(g_htps, "^/v1/queue/settings/(.*)", cb_htp_queue_settings_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/queue/settings$", cb_htp_queue_settings, NULL);

  // statuses
  evhtp_set_regex_cb(g_htps, "^/v1/queue/statuses/(.*)", cb_htp_queue_statuses_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/queue/statuses$", cb_htp_queue_statuses, NULL);



  //// ^/sip/
  // config
  evhtp_set_regex_cb(g_htps, "^/v1/sip/config$", cb_htp_sip_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/v1/sip/configs/(.*)", cb_htp_sip_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/sip/configs$", cb_htp_sip_configs, NULL);

  // peers
  evhtp_set_regex_cb(g_htps, "^/v1/sip/peers/(.*)", cb_htp_sip_peers_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/sip/peers$", cb_htp_sip_peers, NULL);

  // registries
  evhtp_set_regex_cb(g_htps, "^/v1/sip/registries/(.*)", cb_htp_sip_registries_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/sip/registries$", cb_htp_sip_registries, NULL);

  // settings
  evhtp_set_regex_cb(g_htps, "^/v1/sip/settings/(.*)", cb_htp_sip_settings_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/sip/settings$", cb_htp_sip_settings, NULL);



  //// ^/user/
  // contacts
  evhtp_set_regex_cb(g_htps, "^/v1/user/contacts/(.*)", cb_htp_user_contacts_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/user/contacts$", cb_htp_user_contacts, NULL);

  // login
  evhtp_set_regex_cb(g_htps, "^/v1/user/login$", cb_htp_user_login, NULL);

  // permissions
  evhtp_set_regex_cb(g_htps, "^/v1/user/permissions/(.*)", cb_htp_user_permissions_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/user/permissions$", cb_htp_user_permissions, NULL);

  // users
  evhtp_set_regex_cb(g_htps, "^/v1/user/users/(.*)", cb_htp_user_users_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/user/users$", cb_htp_user_users, NULL);


  //// ^/voicemail/
  // config
  evhtp_set_regex_cb(g_htps, "^/v1/voicemail/config$", cb_htp_voicemail_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/v1/voicemail/configs/(.*)", cb_htp_voicemail_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/voicemail/configs$", cb_htp_voicemail_configs, NULL);

  // users
  evhtp_set_regex_cb(g_htps, "^/v1/voicemail/users/(.*)", cb_htp_voicemail_users_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/voicemail/users$", cb_htp_voicemail_users, NULL);

  // vms
  evhtp_set_regex_cb(g_htps, "^/v1/voicemail/vms/("DEF_REG_MSGNAME")", cb_htp_voicemail_vms_msgname, NULL);
  evhtp_set_regex_cb(g_htps, "^/v1/voicemail/vms$", cb_htp_voicemail_vms, NULL);



  /////////////////////// apis


  //// ^/agent/
  // agents
  evhtp_set_regex_cb(g_htps, "^/agent/agents/(.*)", cb_htp_agent_agents_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/agent/agents$", cb_htp_agent_agents, NULL);



  //// ^/core/
  // agis
  evhtp_set_regex_cb(g_htps, "^/core/agis/(.*)", cb_htp_core_agis_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/core/agis$", cb_htp_core_agis, NULL);

  // channels
  evhtp_set_regex_cb(g_htps, "^/core/channels/(.*)", cb_htp_core_channels_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/core/channels$", cb_htp_core_channels, NULL);

  // modules
  evhtp_set_regex_cb(g_htps, "^/core/modules/(.*)", cb_htp_core_modules_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/core/modules$", cb_htp_core_modules, NULL);

  // systems
  evhtp_set_regex_cb(g_htps, "^/core/systems/(.*)", cb_htp_core_systems_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/core/systems$", cb_htp_core_systems, NULL);



  //// ^/dp/
  // config
  evhtp_set_regex_cb(g_htps, "^/dp/config$", cb_htp_dp_config, NULL);

  // dialplans
  evhtp_set_regex_cb(g_htps, "^/dp/dialplans/(.*)", cb_htp_dp_dialplans_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/dp/dialplans$", cb_htp_dp_dialplans, NULL);
  // dpmas
  evhtp_set_regex_cb(g_htps, "^/dp/dpmas/(.*)", cb_htp_dp_dpmas_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/dp/dpmas$", cb_htp_dp_dpmas, NULL);



  //// ^/me/
  // buddies
  evhtp_set_regex_cb(g_htps, "^/me/buddies$", cb_htp_me_buddies, NULL);
  evhtp_set_regex_cb(g_htps, "^/me/buddies/("DEF_REG_UUID")$", cb_htp_me_buddies_detail, NULL);

  // chats
  evhtp_set_regex_cb(g_htps, "^/me/chats$", cb_htp_me_chats, NULL);
  evhtp_set_regex_cb(g_htps, "^/me/chats/("DEF_REG_UUID")$", cb_htp_me_chats_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/me/chats/("DEF_REG_UUID")/messages$", cb_htp_me_chats_detail_messages, NULL);

  // info
  evhtp_set_regex_cb(g_htps, "^/me/info$", cb_htp_me_info, NULL);

  // login
  evhtp_set_regex_cb(g_htps, "^/me/login$", cb_htp_me_login, NULL);



  ////// ^/ob/
  ////// outbound modules
  // destinations
  evhtp_set_regex_cb(g_htps, "^/ob/destinations$", ob_cb_htp_ob_destinations, NULL);
  evhtp_set_regex_cb(g_htps, "^/ob/destinations/("DEF_REG_UUID")$", ob_cb_htp_ob_destinations_detail, NULL);

  // plans
  evhtp_set_regex_cb(g_htps, "^/ob/plans$", ob_cb_htp_ob_plans, NULL);
  evhtp_set_regex_cb(g_htps, "^/ob/plans/("DEF_REG_UUID")$", ob_cb_htp_ob_plans_detail, NULL);

  // campaigns
  evhtp_set_regex_cb(g_htps, "^/ob/campaigns$", ob_cb_htp_ob_campaigns, NULL);
  evhtp_set_regex_cb(g_htps, "^/ob/campaigns/("DEF_REG_UUID")$", ob_cb_htp_ob_campaigns_detail, NULL);

  // dlmas
  evhtp_set_regex_cb(g_htps, "^/ob/dlmas$", ob_cb_htp_ob_dlmas, NULL);
  evhtp_set_regex_cb(g_htps, "^/ob/dlmas/("DEF_REG_UUID")$", ob_cb_htp_ob_dlmas_detail, NULL);

  // dls
  evhtp_set_regex_cb(g_htps, "^/ob/dls$", ob_cb_htp_ob_dls, NULL);
  evhtp_set_regex_cb(g_htps, "^/ob/dls/("DEF_REG_UUID")$", ob_cb_htp_ob_dls_detail, NULL);

  // dialings
  evhtp_set_regex_cb(g_htps, "^/ob/dialings$", ob_cb_htp_ob_dialings, NULL);
  evhtp_set_regex_cb(g_htps, "^/ob/dialings/("DEF_REG_UUID")$", ob_cb_htp_ob_dialings_detail, NULL);



  //// ^/park/
  // config
  evhtp_set_regex_cb(g_htps, "^/park/config$", cb_htp_park_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/park/configs/(.*)", cb_htp_park_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/park/configs$", cb_htp_park_configs, NULL);

  // parkinglots
  evhtp_set_regex_cb(g_htps, "^/park/parkinglots/(.*)", cb_htp_park_parkinglots_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/park/parkinglots$", cb_htp_park_parkinglots, NULL);

  // parkedcalls
  evhtp_set_regex_cb(g_htps, "^/park/parkedcalls/(.*)", cb_htp_park_parkedcalls_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/park/parkedcalls$", cb_htp_park_parkedcalls, NULL);

  // settings
  evhtp_set_regex_cb(g_htps, "^/park/settings/(.*)", cb_htp_park_settings_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/park/settings$", cb_htp_park_settings, NULL);


  //// ^/pjsip/
  // aors
  evhtp_set_regex_cb(g_htps, "^/pjsip/aors/(.*)", cb_htp_pjsip_aors_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/aors$", cb_htp_pjsip_aors, NULL);

  // auths
  evhtp_set_regex_cb(g_htps, "^/pjsip/auths/(.*)", cb_htp_pjsip_auths_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/auths$", cb_htp_pjsip_auths, NULL);

  // config
  evhtp_set_regex_cb(g_htps, "^/pjsip/config$", cb_htp_pjsip_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/pjsip/configs/(.*)", cb_htp_pjsip_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/configs$", cb_htp_pjsip_configs, NULL);

  // contacts
  evhtp_set_regex_cb(g_htps, "^/pjsip/contacts/(.*)", cb_htp_pjsip_contacts_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/contacts$", cb_htp_pjsip_contacts, NULL);

  // endpoints
  evhtp_set_regex_cb(g_htps, "^/pjsip/endpoints/(.*)", cb_htp_pjsip_endpoints_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/endpoints$", cb_htp_pjsip_endpoints, NULL);

  // identifies
  evhtp_set_regex_cb(g_htps, "^/pjsip/identifies/(.*)", cb_htp_pjsip_identifies_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/identifies$", cb_htp_pjsip_identifies, NULL);

  // registrations
  evhtp_set_regex_cb(g_htps, "^/pjsip/registrations/(.*)", cb_htp_pjsip_registrations_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/registrations$", cb_htp_pjsip_registrations, NULL);

  // transports
  evhtp_set_regex_cb(g_htps, "^/pjsip/transports/(.*)", cb_htp_pjsip_transports_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/transports$", cb_htp_pjsip_transports, NULL);



  //// ^/queue/
  // config
  evhtp_set_regex_cb(g_htps, "^/queue/config$", cb_htp_queue_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/queue/configs/(.*)", cb_htp_queue_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/queue/configs$", cb_htp_queue_configs, NULL);

  // entries
  evhtp_set_regex_cb(g_htps, "^/queue/entries/(.*)", cb_htp_queue_entries_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/queue/entries$", cb_htp_queue_entries, NULL);

  // members
  evhtp_set_regex_cb(g_htps, "^/queue/members/(.*)", cb_htp_queue_members_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/queue/members$", cb_htp_queue_members, NULL);

  // queues
  evhtp_set_regex_cb(g_htps, "^/queue/queues/(.*)", cb_htp_queue_queues_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/queue/queues$", cb_htp_queue_queues, NULL);

  // settings
  evhtp_set_regex_cb(g_htps, "^/queue/settings/(.*)", cb_htp_queue_settings_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/queue/settings$", cb_htp_queue_settings, NULL);

  // statuses
  evhtp_set_regex_cb(g_htps, "^/queue/statuses/(.*)", cb_htp_queue_statuses_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/queue/statuses$", cb_htp_queue_statuses, NULL);



  //// ^/sip/
  // config
  evhtp_set_regex_cb(g_htps, "^/sip/config$", cb_htp_sip_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/sip/configs/(.*)", cb_htp_sip_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/sip/configs$", cb_htp_sip_configs, NULL);

  // peers
  evhtp_set_regex_cb(g_htps, "^/sip/peers/(.*)", cb_htp_sip_peers_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/sip/peers$", cb_htp_sip_peers, NULL);

  // registries
  evhtp_set_regex_cb(g_htps, "^/sip/registries/(.*)", cb_htp_sip_registries_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/sip/registries$", cb_htp_sip_registries, NULL);

  // settings
  evhtp_set_regex_cb(g_htps, "^/sip/settings/(.*)", cb_htp_sip_settings_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/sip/settings$", cb_htp_sip_settings, NULL);



  //// ^/user/
  // contacts
  evhtp_set_regex_cb(g_htps, "^/user/contacts/(.*)", cb_htp_user_contacts_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/user/contacts$", cb_htp_user_contacts, NULL);

  // login
  evhtp_set_regex_cb(g_htps, "^/user/login$", cb_htp_user_login, NULL);

  // permissions
  evhtp_set_regex_cb(g_htps, "^/user/permissions/(.*)", cb_htp_user_permissions_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/user/permissions$", cb_htp_user_permissions, NULL);

  // users
  evhtp_set_regex_cb(g_htps, "^/user/users/(.*)", cb_htp_user_users_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/user/users$", cb_htp_user_users, NULL);


  //// ^/voicemail/
  // config
  evhtp_set_regex_cb(g_htps, "^/voicemail/config$", cb_htp_voicemail_config, NULL);

  // configs
  evhtp_set_regex_cb(g_htps, "^/voicemail/configs/(.*)", cb_htp_voicemail_configs_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/voicemail/configs$", cb_htp_voicemail_configs, NULL);

  // users
  evhtp_set_regex_cb(g_htps, "^/voicemail/users/(.*)", cb_htp_voicemail_users_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/voicemail/users$", cb_htp_voicemail_users, NULL);

  // vms
  evhtp_set_regex_cb(g_htps, "^/voicemail/vms/("DEF_REG_MSGNAME")", cb_htp_voicemail_vms_msgname, NULL);
  evhtp_set_regex_cb(g_htps, "^/voicemail/vms$", cb_htp_voicemail_vms, NULL);




  // old apis.
  // will be deprecated.

  // pjsip/settings
  evhtp_set_regex_cb(g_htps, "^/pjsip/settings/(.*)", cb_htp_pjsip_settings_detail, NULL);
  evhtp_set_regex_cb(g_htps, "^/pjsip/settings$", cb_htp_pjsip_settings, NULL);


  // register callback
  evhtp_set_cb(g_htps, "/ping", cb_htp_ping, NULL);

  // peers
  evhtp_set_cb(g_htps, "/peers/", cb_htp_sip_peers_detail, NULL);
  evhtp_set_cb(g_htps, "/peers", cb_htp_sip_peers, NULL);

  // databases - deprecated
  evhtp_set_cb(g_htps, "/databases/", cb_htp_databases_key, NULL);
  evhtp_set_cb(g_htps, "/databases", cb_htp_databases, NULL);

  // registres
  evhtp_set_cb(g_htps, "/registries/", cb_htp_sip_registries_detail, NULL);
  evhtp_set_cb(g_htps, "/registries", cb_htp_sip_registries, NULL);

  // queue_params
  evhtp_set_cb(g_htps, "/queue_params/", cb_htp_queue_queues_detail, NULL);
  evhtp_set_cb(g_htps, "/queue_params", cb_htp_queue_queues, NULL);

  // queue_members
  evhtp_set_cb(g_htps, "/queue_members/", cb_htp_queue_members_detail, NULL);
  evhtp_set_cb(g_htps, "/queue_members", cb_htp_queue_members, NULL);

  // queue_entries
  evhtp_set_cb(g_htps, "/queue_entries/", cb_htp_queue_entries_detail, NULL);
  evhtp_set_cb(g_htps, "/queue_entries", cb_htp_queue_entries, NULL);

  // queue_entries
  evhtp_set_cb(g_htps, "/channels/", cb_htp_core_channels_detail, NULL);
  evhtp_set_cb(g_htps, "/channels", cb_htp_core_channels, NULL);

  // agents
  evhtp_set_cb(g_htps, "/agents/", cb_htp_agent_agents_detail, NULL);
  evhtp_set_cb(g_htps, "/agents", cb_htp_agent_agents, NULL);

  // systems
  evhtp_set_cb(g_htps, "/systems/", cb_htp_core_systems_detail, NULL);
  evhtp_set_cb(g_htps, "/systems", cb_htp_core_systems, NULL);

  // device_states
  evhtp_set_cb(g_htps, "/device_states/", cb_htp_device_states_detail, NULL);
  evhtp_set_cb(g_htps, "/device_states", cb_htp_device_states, NULL);

  // parking_lots
  evhtp_set_cb(g_htps, "/parking_lots/", cb_htp_park_parkinglots_detail, NULL);
  evhtp_set_cb(g_htps, "/parking_lots", cb_htp_park_parkinglots, NULL);

  // parked_calls
  evhtp_set_cb(g_htps, "/parked_calls/", cb_htp_park_parkedcalls_detail, NULL);
  evhtp_set_cb(g_htps, "/parked_calls", cb_htp_park_parkedcalls, NULL);


  return true;
}

void http_term_handler(void)
{
  slog(LOG_INFO, "Terminate http handler.");
  if(g_htps != NULL) {
    evhtp_unbind_socket(g_htps);
    evhtp_free(g_htps);
  }
}

static bool init_https(void)
{
  const char* tmp_const;
  const char* addr;
  evhtp_ssl_cfg_t scfg;
  int port;
  int ret;

  g_htps = evhtp_new(g_app->evt_base, NULL);

  // get https info
  addr = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "https_addr"));
  if(addr == NULL) {
    slog(LOG_ERR, "Could not get https_addr info.");
    return false;
  }
  tmp_const = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "https_port"));
  port = atoi(tmp_const);

  // ssl options
  memset(&scfg, 0x00, sizeof(scfg));
  scfg.pemfile = (char*)json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "https_pemfile"));

  // init ssl
  ret = evhtp_ssl_init(g_htps, &scfg);
  if(ret == -1) {
    slog(LOG_ERR, "Could not initiate https.");
    return false;
  }

  slog(LOG_INFO, "The https server info. addr[%s], port[%d]", addr, port);
  ret = evhtp_bind_socket(g_htps, addr, port, 1024);
  if(ret != 0) {
    slog(LOG_ERR, "Could not open the https server.");
    return false;
  }

  return true;
}

/**
 * Get userinfo of given request
 * @param req
 * @return
 */
json_t* http_get_userinfo(evhtp_request_t *req)
{
  char* token;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get token
  token = http_get_authtoken(req);
  if(token == NULL) {
    return NULL;
  }

  // get user info
  j_res = user_get_userinfo_by_authtoken(token);
  sfree(token);
  if(j_res == NULL) {
    return NULL;
  }

  return j_res;
}

void http_simple_response_normal(evhtp_request_t *req, json_t* j_msg)
{
  char* res;

  if((req == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired simple_response_normal.");

  // add default headers
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Headers", "x-requested-with, content-type, accept, origin, authorization", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Origin", "*", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Max-Age", "86400", 1, 1));

  res = json_dumps(j_msg, JSON_ENCODE_ANY);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, EVHTP_RES_OK);
  sfree(res);

  return;
}

void http_simple_response_error(evhtp_request_t *req, int status_code, int err_code, const char* err_msg)
{
  char* res;
  json_t* j_tmp;
  json_t* j_res;
  int code;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired simple_response_error.");

  // add default headers
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Headers", "x-requested-with, content-type, accept, origin, authorization", 1, 1));
  evhtp_headers_add_header (req->headers_out, evhtp_header_new("Access-Control-Allow-Methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Origin", "*", 1, 1));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Max-Age", "86400", 1, 1));

  if(evhtp_request_get_method(req) == htp_method_OPTIONS) {
    evhtp_send_reply(req, EVHTP_RES_OK);
    return;
  }

  // create default result
  j_res = http_create_default_result(status_code);

  // create error
  if(err_code == 0) {
    code = status_code;
  }
  else {
    code = err_code;
  }
  j_tmp = json_pack("{s:i, s:s}",
      "code",     code,
      "message",  err_msg? : ""
      );
  json_object_set_new(j_res, "error", j_tmp);

  res = json_dumps(j_res, JSON_ENCODE_ANY);
  json_decref(j_res);

  evbuffer_add_printf(req->buffer_out, "%s", res);
  evhtp_send_reply(req, status_code);
  sfree(res);

  return;
}

json_t* http_create_default_result(int code)
{
  json_t* j_res;
  char* timestamp;

  timestamp = utils_get_utc_timestamp();

  j_res = json_pack("{s:s, s:s, s:i}",
      "api_ver",    API_VER,
      "timestamp",  timestamp,
      "statuscode", code
      );
  sfree(timestamp);

  return j_res;
}

/**
 * Get data from request
 * @param req
 * @return
 */
static char* get_data_from_request(evhtp_request_t* req)
{
  const char* tmp_const;
  char* tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get data
  tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
  if(tmp_const == NULL) {
    return NULL;
  }

  // copy data
  tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
  slog(LOG_DEBUG, "Requested data. data[%s]", tmp);

  return tmp;
}

/**
 * Get data json format
 * @param req
 * @return
 */
json_t* http_get_json_from_request_data(evhtp_request_t* req)
{
  char* tmp;
  json_t* j_data;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get data
  tmp = get_data_from_request(req);
  if(tmp == NULL) {
    return NULL;
  }

  // create json
  j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
  sfree(tmp);
  if(j_data == NULL) {
    return NULL;
  }

  return j_data;
}

/**
 * Get data text format
 * @param req
 * @return
 */
char* http_get_text_from_request_data(evhtp_request_t* req)
{
  char* tmp;

  tmp = get_data_from_request(req);

  return tmp;
}

/**
 *
 * @param req
 * @param agent_uuid      (out) agent id
 * @param agent_pass    (out) agent pass
 * @return
 */
bool http_get_htp_id_pass(evhtp_request_t* req, char** agent_uuid, char** agent_pass)
{
  evhtp_connection_t* conn;
  char *auth_hdr, *auth_b64;
  char *outstr;
  char username[1024], password[1024];
  int  i;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  conn = evhtp_request_get_connection(req);
  if(conn == NULL) {
    slog(LOG_NOTICE, "Could not get correct evhtp_connection info.");
    return false;
  }

  *agent_uuid = NULL;
  *agent_pass = NULL;

  // get Authorization
  if((conn->request->headers_in == NULL)
      || ((auth_hdr = (char*)evhtp_kv_find(conn->request->headers_in, "Authorization")) == NULL)
      ) {
    slog(LOG_WARNING, "Could not find Authorization header.");
    return false;
  }

  // decode base_64
  auth_b64 = auth_hdr;
  while(*auth_b64++ != ' ');  // Something likes.. "Basic cGNoZXJvOjEyMzQ="

  base64decode(auth_b64, &outstr);
  if(outstr == NULL) {
    slog(LOG_ERR, "Could not decode base64. info[%s]", auth_b64);
    return false;
  }
  slog(LOG_DEBUG, "Decoded userinfo. info[%s]", outstr);

  // parsing user:pass
  for(i = 0; i < strlen(outstr); i++) {
    if(outstr[i] == ':') {
      break;
    }
    username[i] = outstr[i];
  }
  username[i] = '\0';
  strncpy(password, outstr + i + 1, sizeof(password));
  free(outstr);
  slog(LOG_DEBUG, "User info. user[%s], pass[%s]", username, password);

  asprintf(agent_uuid, "%s", username);
  asprintf(agent_pass, "%s", password);

  return true;
}

/**
 * Return the parsed detail info.
 * Need to release by the caller.
 * @param req
 * @return
 */
char* http_get_parsed_detail(evhtp_request_t* req)
{
  const char* tmp_const;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // detail parse
  tmp_const = req->uri->path->file;
  detail = utils_uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not decode detail info.");
    return NULL;
  }

  return detail;
}

/**
 * Return the parsed start detail info.
 * Need to release by the caller.
 * @param req
 * @return
 */
char* http_get_parsed_detail_start(evhtp_request_t* req)
{
  const char* tmp_const;
  char* detail;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  tmp_const = req->uri->path->match_start;
  if(tmp_const == NULL) {
    slog(LOG_WARNING, "Could not get detail match_start info.");
    return NULL;
  }
  slog(LOG_DEBUG, "Check value. detail[%s]", tmp_const);

  detail = utils_uri_decode(tmp_const);
  if(detail == NULL) {
    slog(LOG_ERR, "Could not decode match start info.");
    return NULL;
  }

  return detail;
}

/**
 * Get given request's authtoken.
 * @param req
 * @return
 */
char* http_get_authtoken(evhtp_request_t* req)
{
  char* res;
  const char* tmp_const;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get authtoken
  tmp_const = evhtp_kv_find(req->uri->query, "authtoken");
  if(tmp_const == NULL) {
    slog(LOG_NOTICE, "Could not get authtoken info.");
    return NULL;
  }

  res = strdup(tmp_const);
  return res;
}

/**
 * Get given key's value from request.
 * The return string is uri decoded.
 * @param req
 * @param key
 * @return
 */
char* http_get_parameter(evhtp_request_t* req, const char* key)
{
  char* res;
  const char* tmp_const;

  if((req == NULL) || (key == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get key value
  tmp_const = evhtp_kv_find(req->uri->query, key);
  if(tmp_const == NULL) {
    slog(LOG_NOTICE, "Could not get key value.");
    return NULL;
  }

  res = utils_uri_decode(tmp_const);
  return res;
}

/**
 * Check the request has given permission.
 * @param req
 * @param perm
 * @return true: has permission, false: has no permission.
 */
bool http_is_request_has_permission(evhtp_request_t *req, enum EN_HTTP_PERMS perm)
{
  json_t* j_user;
  json_t* j_perm;
  const char* user_uuid;
  const char* permission;
  char* token;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired http_is_request_has_permission. perm[%u]", perm);

  // get permission string
  switch(perm) {
    case EN_HTTP_PERM_ADMIN: {
      permission = DEF_USER_PERM_ADMIN;
    }
    break;

    case EN_HTTP_PERM_USER: {
      permission = DEF_USER_PERM_USER;
    }
    break;

    default: {
      slog(LOG_ERR, "Could not find correct permission string. perm[%u]", perm);
      permission = NULL;
    }
    break;
  }
  if(permission == NULL) {
    slog(LOG_ERR, "Could not get permission string.");
    return false;
  }

  token = http_get_authtoken(req);
  if(token == NULL) {
    slog(LOG_ERR, "Could not get authtoken.");
    return false;
  }

  // update tm_update
  ret = user_update_authtoken_tm_update(token);
  if(ret == false) {
    slog(LOG_ERR, "Could not update tm_update.");
    sfree(token);
    return false;
  }

  j_user = user_get_userinfo_by_authtoken(token);
  sfree(token);
  if(j_user == NULL) {
    slog(LOG_ERR, "Could not get userinfo.");
    return false;
  }

  user_uuid = json_string_value(json_object_get(j_user, "uuid"));
  j_perm = user_get_permission_info_by_useruuid_perm(user_uuid, permission);
  json_decref(j_user);
  if(j_perm == NULL) {
    return false;
  }

  json_decref(j_perm);
  return true;
}

/**
 * http request handler
 * ^/ping
 * @param req
 * @param data
 */
static void cb_htp_ping(evhtp_request_t *req, void *a)
{
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_ping.");

  // create result
  j_tmp = json_pack("{s:s}",
      "message",  "pong"
      );

  j_res = http_create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // send response
  http_simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * http request handler
 * ^/sip/config$
 * @param req
 * @param data
 */
static void cb_htp_sip_config(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_config.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    sip_htp_get_sip_config(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    sip_htp_put_sip_config(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/sip/configs$
 * @param req
 * @param data
 */
static void cb_htp_sip_configs(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_configs.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    sip_htp_get_sip_configs(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/sip/configs/(.*)
 * @param req
 * @param data
 */
static void cb_htp_sip_configs_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_configs_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    sip_htp_get_sip_configs_detail(req, data);
    return;
  }
  else if (method == htp_method_DELETE) {
    sip_htp_delete_sip_configs_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/sip/peers
 * @param req
 * @param data
 */
static void cb_htp_sip_peers(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_peers.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    sip_htp_get_sip_peers(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/peers/
 * @param req
 * @param data
 */
static void cb_htp_sip_peers_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_peers_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    sip_htp_get_sip_peers_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}


/**
 * http request handler
 * ^/databases
 * @param req
 * @param data
 */
static void cb_htp_databases(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_res;
  json_t* j_tmp;

  slog(LOG_WARNING, "Deprecated api.");
  http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_databases.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    j_tmp = get_databases_all_key();
    if(j_tmp == NULL) {
      http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = http_create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", j_tmp);

    http_simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  return;
}

/**
 * http request handler
 * ^/databases/
 * @param req
 * @param data
 */
static void cb_htp_databases_key(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_data;
  json_t* j_res;
  json_t* j_tmp;
  const char* tmp_const;
  char* tmp;

  slog(LOG_WARNING, "Deprecated api.");
  http_simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_databases_key.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    // get data
    tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
    if(tmp_const == NULL) {
      http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // create json
    tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
    slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
    j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
    sfree(tmp);
    if(j_data == NULL) {
      http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // get value
    j_tmp = get_database_info(json_string_value(json_object_get(j_data, "key")));
    json_decref(j_data);
    if(j_tmp == NULL) {
      http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = http_create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", j_tmp);

    http_simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  return;
}

/**
 * http request handler
 * ^/registries
 * @param req
 * @param data
 */
static void cb_htp_sip_registries(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_registries.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    sip_htp_get_sip_registries(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/registries/
 * @param req
 * @param data
 */
static void cb_htp_sip_registries_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_registries_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    sip_htp_get_sip_registries_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/sip/settings$
 * @param req
 * @param data
 */
static void cb_htp_sip_settings(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_settings.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    sip_htp_get_sip_settings(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    sip_htp_post_sip_settings(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/sip/settings/detail
 * @param req
 * @param data
 */
static void cb_htp_sip_settings_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_sip_settings_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    sip_htp_get_sip_settings_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    sip_htp_put_sip_settings_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    queue_htp_delete_queue_settings_detail(req,data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}


/**
 * http request handler
 * ^/queue/queues$
 * @param req
 * @param data
 */
static void cb_htp_queue_queues(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_queues.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_queues(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    queue_htp_post_queue_queues(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/queues/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_queues_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_queues_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_queues_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    queue_htp_put_queue_queues_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    queue_htp_delete_queue_queues_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/config$
 * @param req
 * @param data
 */
static void cb_htp_queue_config(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_config.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_config(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    queue_htp_put_queue_config(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/configs/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_configs_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_configs_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_configs_detail(req, data);
    return;
  }
  else if (method == htp_method_DELETE) {
    queue_htp_delete_queue_configs_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/configs$
 * @param req
 * @param data
 */
static void cb_htp_queue_configs(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_configs.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    queue_htp_get_queue_configs(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/statuses$
 * @param req
 * @param data
 */
static void cb_htp_queue_statuses(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_statuses.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_statuses(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/statuses/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_statuses_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_statuses_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_statuses_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}


/**
 * http request handler
 * ^/queue_members
 * @param req
 * @param data
 */
static void cb_htp_queue_members(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_members.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_members(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/members/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_members_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_members_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_members_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/entries$
 * @param req
 * @param data
 */
static void cb_htp_queue_entries(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_entries.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_entries(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/queue/entries/(.*)
 * @param req
 * @param data
 */
static void cb_htp_queue_entries_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_entries_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_entries_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    // synonym of delete /core/channels/<detail>
    core_htp_delete_core_channels_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/settings$
 * @param req
 * @param data
 */
static void cb_htp_queue_settings(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_settings.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    queue_htp_get_queue_settings(req, data);
    return;
  }
  else if(method == htp_method_POST) {
  	queue_htp_post_queue_settings(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/queue/settings/detail
 * @param req
 * @param data
 */
static void cb_htp_queue_settings_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_queue_settings_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
  	queue_htp_get_queue_settings_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
  	queue_htp_put_queue_settings_detail(req, data);
  	return;
  }
  else if(method == htp_method_DELETE) {
  	queue_htp_delete_queue_settings_detail(req,data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/agis
 * @param req
 * @param data
 */
static void cb_htp_core_agis(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_agis.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    core_htp_get_core_agis(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/agis/<detail>
 * @param req
 * @param data
 */
static void cb_htp_core_agis_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_agis_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    core_htp_get_core_agis_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    // synonym of delete /core/channels/<detail>
    core_htp_delete_core_channels_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/channels
 * @param req
 * @param data
 */
static void cb_htp_core_channels(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_channels.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    core_htp_get_core_channels(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/channels/
 * @param req
 * @param data
 */
static void cb_htp_core_channels_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_channels_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    core_htp_get_core_channels_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    core_htp_delete_core_channels_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/modules$
 * @param req
 * @param data
 */
static void cb_htp_core_modules(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_modules.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    core_htp_get_core_modules(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/core/modules/(.*)
 * @param req
 * @param data
 */
static void cb_htp_core_modules_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_modules_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    core_htp_get_core_modules_detail(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    core_htp_post_core_modules_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    core_htp_put_core_modules_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    core_htp_delete_core_modules_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}


/**
 * http request handler
 * ^/agents
 * @param req
 * @param data
 */
static void cb_htp_agent_agents(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_agents.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    agent_htp_get_agent_agents(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;

}

/**
 * http request handler
 * ^/agent/agents/(id)
 * @param req
 * @param data
 */
static void cb_htp_agent_agents_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_agents_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    agent_htp_get_agent_agents_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/core/systems
 * @param req
 * @param data
 */
static void cb_htp_core_systems(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_systems.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    core_htp_get_core_systems(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;

}

/**
 * http request handler
 * ^/core/systems/
 * @param req
 * @param data
 */
static void cb_htp_core_systems_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_core_systems_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    core_htp_get_core_systems_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/device_states
 * @param req
 * @param data
 */
static void cb_htp_device_states(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_res;
  json_t* j_tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_device_states.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    j_tmp = get_device_states_all_device();
    if(j_tmp == NULL) {
      http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = http_create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", json_object());
    json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

    http_simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/device_states/
 * @param req
 * @param data
 */
static void cb_htp_device_states_detail(evhtp_request_t *req, void *data)
{
  int method;
  json_t* j_data;
  json_t* j_res;
  json_t* j_tmp;
  const char* tmp_const;
  char* tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_systems_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    // get data
    tmp_const = (char*)evbuffer_pullup(req->buffer_in, evbuffer_get_length(req->buffer_in));
    if(tmp_const == NULL) {
      http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // create json
    tmp = strndup(tmp_const, evbuffer_get_length(req->buffer_in));
    slog(LOG_DEBUG, "Requested data. data[%s]", tmp);
    j_data = json_loads(tmp, JSON_DECODE_ANY, NULL);
    sfree(tmp);
    if(j_data == NULL) {
      http_simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
      return;
    }

    // get value
    j_tmp = get_device_state_info(
        json_string_value(json_object_get(j_data, "device"))
        );
    json_decref(j_data);
    if(j_tmp == NULL) {
      http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
      return;
    }

    // create result
    j_res = http_create_default_result(EVHTP_RES_OK);
    json_object_set_new(j_res, "result", j_tmp);

    http_simple_response_normal(req, j_res);
    json_decref(j_res);
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  return;
}

/**
 * http request handler
 * ^/park/parking_lots$
 * @param req
 * @param data
 */
static void cb_htp_park_parkinglots(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parking_lots.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    park_htp_get_park_parkinglots(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    park_htp_post_park_parkinglots(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/park/parking_lots/(.*)
 * @param req
 * @param data
 */
static void cb_htp_park_parkinglots_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parking_lots_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    park_htp_get_park_parkinglots_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    park_htp_put_park_parkinglots_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    park_htp_delete_park_parkinglots_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/park/parkedcalls$
 * @param req
 * @param data
 */
static void cb_htp_park_parkedcalls(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parkedcalls.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    park_htp_get_park_parkedcalls(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/park/parkedcalls/(.*)
 * @param req
 * @param data
 */
static void cb_htp_park_parkedcalls_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_parkedcalls_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    park_htp_get_park_parkedcalls_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    // synonym of delete /core/channels/<detail>
    core_htp_delete_core_channels_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/park/config$
 * @param req
 * @param data
 */
static void cb_htp_park_config(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_config.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    park_htp_get_park_config(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    park_htp_put_park_config(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/park/configs/(.*)
 * @param req
 * @param data
 */
static void cb_htp_park_configs_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_configs_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    park_htp_get_park_configs_detail(req, data);
    return;
  }
  else if (method == htp_method_DELETE) {
    park_htp_delete_park_configs_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/park/configs$
 * @param req
 * @param data
 */
static void cb_htp_park_configs(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_configs.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    park_htp_get_park_configs(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/park/settings$
 * @param req
 * @param data
 */
static void cb_htp_park_settings(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_settings.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    park_htp_get_park_settings(req, data);
    return;
  }
  else if(method == htp_method_POST) {
  	park_htp_post_park_settings(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/park/settings/detail
 * @param req
 * @param data
 */
static void cb_htp_park_settings_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_park_settings_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
  	park_htp_get_park_settings_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
  	park_htp_put_park_settings_detail(req, data);
  	return;
  }
  else if(method == htp_method_DELETE) {
  	park_htp_delete_park_settings_detail(req,data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}


/**
 * http request handler
 * ^/voicemail/users
 * @param req
 * @param data
 */
static void cb_htp_voicemail_users(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_users.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    voicemail_htp_get_voicemail_users(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    voicemail_htp_post_voicemail_users(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/config$
 * @param req
 * @param data
 */
static void cb_htp_voicemail_config(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_config.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    voicemail_htp_get_voicemail_config(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    voicemail_htp_put_voicemail_config(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/configs$
 * @param req
 * @param data
 */
static void cb_htp_voicemail_configs(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_configs.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    voicemail_htp_get_voicemail_configs(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/configs/(.*)
 * @param req
 * @param data
 */
static void cb_htp_voicemail_configs_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_configs_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    voicemail_htp_get_voicemail_configs_detail(req, data);
    return;
  }
  else if (method == htp_method_DELETE) {
    voicemail_htp_delete_voicemail_configs_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/users/<detail>
 * @param req
 * @param data
 */
static void cb_htp_voicemail_users_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_users_context_mailbox.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    voicemail_htp_get_voicemail_users_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    voicemail_htp_put_voicemail_users_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    voicemail_htp_delete_voicemail_users_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}


/**
 * http request handler
 * ^/voicemail/vms
 * @param req
 * @param data
 */
static void cb_htp_voicemail_vms(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_vms.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    voicemail_htp_get_voicemail_vms(req, NULL);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/voicemail/vms/<msgname>
 * @param req
 * @param data
 */
static void cb_htp_voicemail_vms_msgname(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_voicemail_vms_msgname.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    voicemail_htp_get_voicemail_vms_msgname(req, NULL);
    return;
  }
  else if(method == htp_method_DELETE) {
    voicemail_htp_delete_voicemail_vms_msgname(req, NULL);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/pjsip/endpoints
 * @param req
 * @param data
 */
static void cb_htp_pjsip_endpoints(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_endpoints.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_endpoints(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_endpoints(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/endpoints/(*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_endpoints_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_endpoints_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_endpoints_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_endpoints_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_endpoints_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/identifies
 * @param req
 * @param data
 */
static void cb_htp_pjsip_identifies(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_identifies.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    // todo: not support now
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_identifies(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/identifies/(*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_identifies_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_identifies_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    // todo: not support now
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_identifies_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_identifies_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/registrations
 * @param req
 * @param data
 */
static void cb_htp_pjsip_registrations(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_registrations.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    // todo: not support now
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_registrations(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/registrations/(*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_registrations_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_registrations_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    // todo: not support now
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_registrations_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_registrations_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/transports
 * @param req
 * @param data
 */
static void cb_htp_pjsip_transports(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_transports.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    // todo: not support now
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_transports(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/transports/(*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_transports_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_transports_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    // todo: not support now
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_transports_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_transports_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/aors$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_aors(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_aors.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_aors(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_aors(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/aors/(.*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_aors_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_aors_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_aors_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_aors_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_aors_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/auths$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_auths(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_auths.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_auths(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_auths(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/auths/(.*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_auths_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_auths_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_auths_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_auths_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_auths_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/config$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_config(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_config.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_config(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_config(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/pjsip/configs$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_configs(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_configs.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_configs(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/pjsip/configs/(.*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_configs_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_configs_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_configs_detail(req, data);
    return;
  }
  else if (method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_configs_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/pjsip/settings/detail
 * @param req
 * @param data
 */
static void cb_htp_pjsip_settings_detail(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_settings_detail.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_settings_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_settings_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_settings_detail(req,data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/pjsip/settings$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_settings(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_settings.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_settings(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_settings(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/contacts$
 * @param req
 * @param data
 */
static void cb_htp_pjsip_contacts(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_contacts.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_contacts(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    pjsip_htp_post_pjsip_contacts(req, data);
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/pjsip/contacts/(.*)
 * @param req
 * @param data
 */
static void cb_htp_pjsip_contacts_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_pjsip_contacts_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    pjsip_htp_get_pjsip_contacts_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    pjsip_htp_put_pjsip_contacts_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    pjsip_htp_delete_pjsip_contacts_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}


/**
 * http request handler
 * ^/dp/config$
 * @param req
 * @param data
 */
static void cb_htp_dp_config(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_dp_config.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    dialplan_htp_get_dp_config(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    dialplan_htp_put_dp_config(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/dp/dpmas$
 * @param req
 * @param data
 */
static void cb_htp_dp_dpmas(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_dp_dpmas.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    dialplan_htp_get_dp_dpmas(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    dialplan_htp_post_dp_dpmas(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/dp/dpmas/(.*)
 * @param req
 * @param data
 */
static void cb_htp_dp_dpmas_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_dp_dpmas_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    dialplan_htp_get_dp_dpmas_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    dialplan_htp_put_dp_dpmas_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    dialplan_htp_delete_dp_dpmas_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/dp/dialplans$
 * @param req
 * @param data
 */
static void cb_htp_dp_dialplans(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_dp_dialplans.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    dialplan_htp_get_dp_dialplans(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    dialplan_htp_post_dp_dialplans(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/dp/dialplans/(.*)
 * @param req
 * @param data
 */
static void cb_htp_dp_dialplans_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_dp_dialplans_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    dialplan_htp_get_dp_dialplans_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    dialplan_htp_put_dp_dialplans_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    dialplan_htp_delete_dp_dialplans_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/user/login$
 * @param req
 * @param data
 */
static void cb_htp_user_login(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_user_login.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_POST) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_POST) {
    user_htp_post_user_login(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    user_htp_delete_user_login(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/user/permissions$
 * @param req
 * @param data
 */
static void cb_htp_user_permissions(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_user_permissions.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_POST) && (method != htp_method_GET)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_POST) {
    user_htp_post_user_permissions(req, data);
    return;
  }
  else if(method == htp_method_GET) {
    user_htp_get_user_permissions(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/user/permissions/(.*)
 * @param req
 * @param data
 */
static void cb_htp_user_permissions_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_user_permissions_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_DELETE) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if (method == htp_method_DELETE) {
    user_htp_delete_user_permissions_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/user/uesrs$
 * @param req
 * @param data
 */
static void cb_htp_user_users(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_user_users.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_POST) && (method != htp_method_GET)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_POST) {
    user_htp_post_user_users(req, data);
    return;
  }
  else if(method == htp_method_GET) {
    user_htp_get_user_users(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/user/users/(.*)
 * @param req
 * @param data
 */
static void cb_htp_user_users_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_user_users_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    user_htp_get_user_users_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    user_htp_put_user_users_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    user_htp_delete_user_users_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
  return;
}

/**
 * http request handler
 * ^/user/contacts
 * @param req
 * @param data
 */
static void cb_htp_user_contacts(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_user_contacts.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    user_htp_get_user_contacts(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    user_htp_post_user_contacts(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/user/contacts/(.*)
 * @param req
 * @param data
 */
static void cb_htp_user_contacts_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_user_contacts_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    user_htp_get_user_contacts_detail(req, data);
    return;
  }
  else if (method == htp_method_PUT) {
    user_htp_put_user_contacts_detail(req, data);
  }
  else if (method == htp_method_DELETE) {
    user_htp_delete_user_contacts_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/info
 * @param req
 * @param data
 */
static void cb_htp_me_info(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_info.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_info(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    me_htp_put_me_info(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/chats
 * @param req
 * @param data
 */
static void cb_htp_me_chats(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_chats.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_chats(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    me_htp_post_me_chats(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/chats/<detail>
 * @param req
 * @param data
 */
static void cb_htp_me_chats_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_chats_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_chats_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    me_htp_put_me_chats_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    me_htp_delete_me_chats_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/chats/<detail>/messages
 * @param req
 * @param data
 */
static void cb_htp_me_chats_detail_messages(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_chats_detail_messages.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_chats_detail_messages(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    me_htp_post_me_chats_detail_messages(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/me/login
 * @param req
 * @param data
 */
static void cb_htp_me_login(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_login.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_POST) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_POST) {
    me_htp_post_me_login(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    // synonym of delete /user/login
    user_htp_delete_user_login(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/buddies
 * @param req
 * @param data
 */
static void cb_htp_me_buddies(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_buddies.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_buddies(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    me_htp_post_me_buddies(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/buddies/<detail>
 * @param req
 * @param data
 */
static void cb_htp_me_buddies_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_buddies_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_buddies_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    me_htp_put_me_buddies_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    me_htp_delete_me_buddies_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/calls$
 * @param req
 * @param data
 */
static void cb_htp_me_calls(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_calls.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_calls(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    me_htp_post_me_calls(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/calls/<detail>$
 * @param req
 * @param data
 */
static void cb_htp_me_calls_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_calls_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_buddies_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    me_htp_put_me_buddies_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    me_htp_delete_me_buddies_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/me/search$
 * @param req
 * @param data
 */
static void cb_htp_me_search(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_search.");

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    me_htp_get_me_search(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/me/contacts$
 * @param req
 * @param data
 */
static void cb_htp_me_contacts(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_me_contacts.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_USER);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if(method != htp_method_GET) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    me_htp_get_me_contacts(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/admin/login
 * @param req
 * @param data
 */
static void cb_htp_admin_login(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_admin_login.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_POST) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_POST) {
    admin_htp_post_admin_login(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    // synonym of delete /user/login
    user_htp_delete_user_login(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/manager/login
 * @param req
 * @param data
 */
static void cb_htp_manager_login(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_manager_login.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_POST) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_POST) {
    manager_htp_post_manager_login(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    // synonym of delete /user/login
    user_htp_delete_user_login(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler.
 * ^/manager/info
 * @param req
 * @param data
 */
static void cb_htp_manager_info(evhtp_request_t *req, void *data)
{
  int method;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_manager_info.");

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  if(method == htp_method_GET) {
    manager_htp_get_manager_info(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    manager_htp_put_manager_info(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/manager/users$
 * @param req
 * @param data
 */
static void cb_htp_manager_users(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_manager_users.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    manager_htp_get_manager_users(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    manager_htp_post_manager_users(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/manager/users/<detail>
 * @param req
 * @param data
 */
static void cb_htp_manager_users_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_manager_users_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    manager_htp_get_manager_users_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    manager_htp_put_manager_users_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    manager_htp_delete_manager_users_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/manager/trunks$
 * @param req
 * @param data
 */
static void cb_htp_manager_trunks(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_manager_trunks.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_POST)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    manager_htp_get_manager_trunks(req, data);
    return;
  }
  else if(method == htp_method_POST) {
    manager_htp_post_manager_trunks(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

/**
 * http request handler
 * ^/manager/trunks/<detail>
 * @param req
 * @param data
 */
static void cb_htp_manager_trunks_detail(evhtp_request_t *req, void *data)
{
  int method;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_INFO, "Fired cb_htp_manager_trunks_detail.");

  // check authorization
  ret = http_is_request_has_permission(req, EN_HTTP_PERM_ADMIN);
  if(ret == false) {
    http_simple_response_error(req, EVHTP_RES_FORBIDDEN, 0, NULL);
    return;
  }

  // method check
  method = evhtp_request_get_method(req);
  if((method != htp_method_GET) && (method != htp_method_PUT) && (method != htp_method_DELETE)) {
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // fire handlers
  if(method == htp_method_GET) {
    manager_htp_get_manager_trunks_detail(req, data);
    return;
  }
  else if(method == htp_method_PUT) {
    manager_htp_put_manager_trunks_detail(req, data);
    return;
  }
  else if(method == htp_method_DELETE) {
    manager_htp_delete_manager_trunks_detail(req, data);
    return;
  }
  else {
    // should not reach to here.
    http_simple_response_error(req, EVHTP_RES_METHNALLOWED, 0, NULL);
    return;
  }

  // should not reach to here.
  http_simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);

  return;
}

