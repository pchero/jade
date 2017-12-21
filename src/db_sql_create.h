/*
 * db_sql_create.h
 *
 *  Created on: Oct 13, 2016
 *      Author: pchero
 */

#ifndef SRC_DB_SQL_CREATE_H_
#define SRC_DB_SQL_CREATE_H_


// action
static const char* g_sql_drop_action = "drop table if exists action;";
static const char* g_sql_create_action =
"create table action("

// identity
"   id        varchar(255),"
"   type      varchar(255),"
"   data      text,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(id)"

");";


// channel
static const char* g_sql_drop_channel = "drop table if exists channel;";
static const char* g_sql_create_channel =
"create table channel("

// identity
"   unique_id   varchar(255) not null,"
"   linked_id   varchar(255),"

// channel info
"   channel             varchar(255),"    ///< channel name
"   channel_state       int,"
"   channel_state_desc  varchar(255),"

// dial info
"   caller_id_num       varchar(255),"
"   caller_id_name      varchar(255),"

"   connected_line_num  varchar(255),"
"   connected_line_name varchar(255),"

"   language            varchar(255),"
"   account_code        varchar(255),"

// dialplan
"   context     varchar(255),"
"   exten       varchar(255),"
"   priority    varchar(255),"

"   application       varchar(255),"
"   application_data  varchar(1023),"
"   bridge_id         varchar(255),"

"   duration  int,"

// variables
"   variables   text,"

// timestamp. UTC."
"   tm_update     datetime(6),"

"   primary key(unique_id)"
");";


// core_module
static const char* g_sql_drop_core_module = "drop table if exists core_module;";
static const char* g_sql_create_core_module =
"create table core_module("

// identity
"   name      varchar(255) not null,"
"   size      int,"
"   load      varchar(255),"

// timestamp. UTC."
"   tm_update     datetime(6),"

"   primary key(name)"
");";


// peer
static const char* g_sql_drop_peer = "drop table if exists peer;";
static const char* g_sql_create_peer =
"create table peer("

// identity
"   peer         varchar(255)    not null,"
"   status       varchar(255),"
"   address      varchar(255),"     // ip_address:port

// peer info
"   channel_type     varchar(255),"   // channel type(SIP, ...)
"   chan_object_type varchar(255),"   // (peer, ...)
"   monitor_status   varchar(255),"

"   dynamic          varchar(255),"   // dynamic(yes, ...)
"   auto_force_port  varchar(255),"   //
"   force_port       varchar(255),"   //
"   auto_comedia     varchar(255),"   //
"   comedia          varchar(255),"   //
"   video_support    varchar(255),"   //
"   text_support     varchar(255),"   //

"   acl              varchar(255),"   //
"   realtime_device  varchar(255),"   //
"   description      varchar(255),"   //

// timestamp. UTC."
"   tm_update        datetime(6),"   // update time."

"   primary key(peer)"

");";


// queue param
static const char* g_sql_drop_queue_param = "drop table if exists queue_param;";
static const char* g_sql_create_queue_param =
"create table queue_param("

// identity
"   name             varchar(255),"    // queue name.

// status
"   max              int,"             // max available calls in the queue.
"   strategy         varchar(255),"    // queue strategy.
"   calls            int,"             // waiting call count.
"   hold_time        int,"             // average waiting time.
"   talk_time        int,"             // average talk time.
"   completed        int,"             // distributed call count.
"   abandoned        int,"             // could not distributed call count.

// performance
"   service_level       int,"                 // service level interval time sec.
"   service_level_perf  real  default 0.0,"   // service level performance(%). completed_call / (abandoned_call + completed_call) * 100

// etc
"   weight          int,"    // queue priority.

// timestamp. UTC."
"   tm_update       datetime(6),"   // update time."

"   primary key(name)"

");";


// queue member
static const char* g_sql_drop_queue_member = "drop table if exists queue_member;";
static const char* g_sql_create_queue_member =
"create table queue_member("

// identity
"   id             varchar(255),"   // member id(name@queue)
"   queue_name     varchar(255),"   // queue name
"   name           varchar(255),"   // member name

"   location          varchar(255),"          // location
"   state_interface   varchar(255),"          // state interface
"   membership        varchar(255),"  // membership
"   penalty           int,"           // penalty
"   calls_taken       int,"           // call taken count.
"   last_call         int,"
"   last_pause        int,"
"   in_call           int,"
"   status            int,"
"   paused            int,"
"   paused_reason     text,"
"   ring_inuse        int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(queue_name, name)"

");";


// queue entry
static const char* g_sql_drop_queue_entry = "drop table if exists queue_entry;";
static const char* g_sql_create_queue_entry =
"create table queue_entry("

// identity
"   unique_id         varchar(255),"
"   queue_name        varchar(255),"
"   channel           varchar(255),"

// info
"   position            int,"
"   caller_id_num       varchar(255),"
"   caller_id_name      varchar(255),"
"   connected_line_num  varchar(255),"
"   connected_line_name varchar(255),"

"   wait                int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(unique_id)"

");";


// database
static const char* g_sql_drop_database = "drop table if exists database;";
static const char* g_sql_create_database =
"create table database("

// identity
"   key            varchar(255)," // database key
"   value          text,"         // database value

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(key)"

");";

// registry
static const char* g_sql_drop_registry = "drop table if exists registry;";
static const char* g_sql_create_registry =
"create table registry("

// identity
"   account     varchar(255),"
"   username    varchar(255),"
"   host        varchar(255),"
"   port        int,"

// info
"   domain            varchar(255),"
"   domain_port       int,"
"   refresh           int,"
"   state             text,"
"   registration_time int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(account)"

");";


// agent
static const char* g_sql_drop_agent = "drop table if exists agent;";
static const char* g_sql_create_agent =
"create table agent("

// identity
"   id              varchar(255),"
"   status          varchar(255),"
"   name            varchar(255),"
"   logged_in_time  int,"           ///< logged in time.

// channel info
"   channel_name        varchar(255),"
"   channel_state       int,"
"   channel_state_desc  varchar(255),"

// caller info
"   caller_id_num       varchar(255),"
"   caller_id_name      varchar(255),"

// connected line info
"   connected_line_num  varchar(255),"
"   connected_line_name varchar(255),"

"   language      varchar(255),"
"   account_code  varchar(255),"

// dialplan
"   context       varchar(255),"
"   exten         varchar(255),"
"   priority      varchar(255),"

"   unique_id     varchar(255),"
"   linked_id     varchar(255),"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(id)"

");";

// system
static const char* g_sql_drop_system = "drop table if exists system;";
static const char* g_sql_create_system =
"create table system("

// identity
"   id varchar(255),"

// version info
"   ami_version   varchar(255),"
"   ast_version   varchar(255),"
"   system_name   varchar(255),"

//  time info
"   startup_date  varchar(255),"
"   startup_time  varchar(255),"
"   reload_date   varchar(255),"
"   reload_time   varchar(255),"
"   current_calls int,"

// max info
"   max_calls         int,"
"   max_load_avg      real,"
"   max_file_handles  int,"

"   run_user          varchar(255),"
"   run_group         varchar(255),"

"   real_time_enabled varchar(255),"
"   cdr_enabled       varchar(255),"
"   http_enabled      varchar(255),"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(id)"

");";


// device_state
static const char* g_sql_drop_device_state = "drop table if exists device_state;";
static const char* g_sql_create_device_state =
"create table device_state("

// identity
"   device  varchar(255),"

// info
"   state   varchar(255),"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(device)"
");";


// parking_lot
static const char* g_sql_drop_parking_lot = "drop table if exists parking_lot;";
static const char* g_sql_create_parking_lot =
"create table parking_lot("

"   name varchar(255),"

"   start_space varchar(255),"
"   stop_spcae  varchar(255),"

"   timeout int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(name)"
");";


// parked_call
static const char* g_sql_drop_parked_call = "drop table if exists parked_call;";
static const char* g_sql_create_parked_call =
"create table parked_call("

// parked channel id info
"   parkee_unique_id  varchar(255),"
"   parkee_linked_id  varchar(255),"

// parked channel info
"   parkee_channel              varchar(255),"
"   parkee_channel_state        varchar(255),"
"   parkee_channel_state_desc   varchar(255),"

// parked channel caller info
"   parkee_caller_id_num    varchar(255),"
"   parkee_caller_id_name   varchar(255),"

// parked channel connected line info
"   parkee_connected_line_num   varchar(255),"
"   parkee_connected_line_name  varchar(255),"

// parked channel account info
"   parkee_account_code   varchar(255),"

// parked channel dialplan info
"   parkee_context    varchar(255),"
"   parkee_exten      varchar(255),"
"   parkee_priority   varchar(255),"

// parked channel parker info
"   parker_dial_string varchar(255),"

// parking lot info
"   parking_lot       varchar(255),"
"   parking_space     varchar(255),"
"   parking_timeout   int,"
"   parking_duration  int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(parkee_unique_id)"
");";


// pjsip_contact
static const char* g_sql_drop_pjsip_contact = "drop table if exists pjsip_contact;";
static const char* g_sql_create_pjsip_contact =
"create table pjsip_contact("

"   id              varchar(1023),"
"   uri             varchar(1023),"
"   aor             varchar(1023),"
"   endpoint_name   varchar(1023),"

"   status  varchar(255),"

"   round_trip_usec varchar(255),"
"   user_agent      varchar(4095),"
"   reg_expire      int,"
"   via_address     varchar(1023),"
"   call_id         varchar(1023),"

"   authentication_qualify  int,"
"   outbound_proxy          varchar(1023),"
"   path                    varchar(1023),"

"   qualify_frequency   int,"
"   qualify_timout      int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(id)"

");";

// pjsip_endpoint
static const char* g_sql_drop_pjsip_endpoint = "drop table if exists pjsip_endpoint;";
static const char* g_sql_create_pjsip_endpoint =
"create table pjsip_endpoint ("

// basic info
"   object_type     varchar(255),"
"   object_name     varchar(1023),"
"   auth            varchar(1023),"
"   aors            varchar(1023),"

// context
"   context             varchar(255),"
"   message_context     varchar(1023),"
"   subscribe_context   varchar(1023),"

// mailbox
"   mailboxes               varchar(1023),"
"   voicemail_extension     varchar(1023),"
"   incoming_mwi_mailbox    varchar(1023),"

// allow and disallow
"   disallow            varchar(255),"
"   allow               varchar(1023),"
"   allow_subscribe     varchar(1023),"
"   allow_overlap       varchar(1023),"
"   allow_transfer      varchar(1023),"

"   webrtc          varchar(255),"

"   dtmf_mode       varchar(1023),"

// rtp
"   rtp_engine            varchar(1023),"
"   rtp_ipv6              varchar(1023),"
"   rtp_symmetric         varchar(1023),"
"   rtp_keepalive         int,"
"   rtp_timeout           int,"
"   rtp_timeout_hold      int,"
"   asymmetric_rtp_codec  varchar(255),"

"   rtcp_mux        varchar(255),"

"   ice_support     varchar(1023),"
"   use_ptime       varchar(1023),"

"   force_rport         varchar(1023),"
"   rewrite_contact     varchar(1023),"
"   transport           varchar(1023),"
"   moh_suggest         varchar(1023),"

"   rel_100                 varchar(1023),"

// timers
"   timers                  varchar(1023),"
"   timers_min_se           varchar(1023),"
"   timers_sess_expires     int,"

// outbound
"   outbound_proxy          varchar(1023),"
"   outbound_auth           varchar(1023),"

"   identify_by             varchar(1023),"

"   redirect_method         varchar(1023),"

// direct media
"   direct_media                    varchar(1023),"
"   direct_media_method             varchar(1023),"
"   direct_media_glare_mitigation   varchar(1023),"
"   disable_direct_media_on_nat     varchar(1023),"

"   connected_line_method           varchar(1023),"

// caller id
"   caller_id               varchar(1023),"
"   caller_id_privacy       varchar(1023),"
"   caller_id_tag           varchar(1023),"

// trust id
"   trust_id_inbound        varchar(1023),"
"   trust_id_outbound       varchar(1023),"

"   send_pai                varchar(1023),"

"   rpid_immediate          varchar(255),"
"   send_rpid               varchar(1023),"
"   send_diversion          varchar(1023),"

"   aggregate_mwi           varchar(1023),"

// media
"   media_address           varchar(1023),"
"   media_encryption                varchar(1023),"
"   media_encryption_optimistic     varchar(1023),"
"   media_use_received_transport    varchar(1023),"

"   use_avpf                        varchar(1023),"
"   force_avp                       varchar(1023),"

"   one_touch_recording             varchar(1023),"

// prgoress
"   inband_progress         varchar(1023),"
"   refer_blind_progress    varchar(255),"

// group
"   call_group              varchar(1023),"
"   pickup_group            varchar(1023),"
"   named_call_group        varchar(1023),"
"   named_pickup_group      varchar(1023),"

"   device_state            varchar(1023),"
"   device_state_busy_at    int,"

// t38 fax
"   fax_detect              varchar(1023),"
"   fax_detect_time         int,"
"   t38_udptl               varchar(1023),"
"   t38_udptl_ec            varchar(1023),"
"   t38_udptl_maxdatagram   int,"
"   t38_udptl_nat           varchar(1023),"
"   t38_udptl_ipv6          varchar(1023),"

"   tone_zone               varchar(1023),"
"   language                varchar(1023),"

// record
"   record_on_feature       varchar(1023),"
"   record_off_feature      varchar(1023),"

"   user_eq_phone       varchar(1023),"
"   moh_passthrough     varchar(1023),"

// sdp
"   sdp_owner       varchar(1023),"
"   sdp_session     varchar(1023),"

// tos
"   tos_audio       int,"
"   tos_video       int,"

// cos
"   cos_audio       int,"
"   cos_video       int,"

"   sub_min_expiry      varchar(1023),"
"   from_user           varchar(1023),"
"   from_domain         varchar(1023),"

// mwi
"   mwi_from_user                       varchar(1023),"
"   mwi_subscribe_replaces_unsolicited  varchar(255),"

// dtls
"   dtls_verify         varchar(1023),"
"   dtls_rekey          int,"
"   dtls_cert_file      varchar(1023),"
"   dtls_private_key    varchar(1023),"
"   dtls_cipher         varchar(1023),"
"   dtls_ca_file        varchar(1023),"
"   dtls_ca_path        varchar(1023),"
"   dtls_setup          varchar(1023),"
"   dtls_fingerprint    varchar(1023),"

"   srtp_tag32      varchar(1023),"

"   set_var                 varchar(1023),"

"   account_code            varchar(1023),"
"   preferred_codec_only    varchar(1023),"

"   active_channels     varchar(1023),"

// max streams
"   max_audio_streams           int,"
"   max_video_streams           int,"

// acl
"   acl                         varchar(1023),"
"   contact_acl                 varchar(1023),"

// etc
"   g_726_non_standard          varchar(255),"
"   notify_early_inuse_ringing  varchar(255),"
"   bind_rtp_to_media_address   varchar(255),"
"   bundle                      varchar(255),"


// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(object_name)"

");";

// pjsip_aor
static const char* g_sql_drop_pjsip_aor = "drop table if exists pjsip_aor;";
static const char* g_sql_create_pjsip_aor =
"create table pjsip_aor("

// basic info
"   object_type         varchar(255),"
"   object_name         varchar(255),"
"   endpoint_name       varchar(255),"

// expiration
"   minimum_expiration  int,"
"   default_expiration  int,"
"   maximum_expiration  int,"

// qualify
"   qualify_timeout     int,"
"   qualify_frequency   int,"

// contact
"   contacts                    string,"
"   max_contacts                int,"
"   total_contacts              int,"
"   contacts_registered         int,"
"   remove_existing             varchar(255),"

// etc
"   mailboxes                   varchar(1023),"
"   support_path                varchar(255),"
"   voicemail_extension         varchar(1023),"
"   authenticate_qualify        varchar(255),"
"   outbound_proxy              varchar(1023),"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary  key(object_name)"

");";

// pjsip_aor
static const char* g_sql_drop_pjsip_auth = "drop table if exists pjsip_auth;";
static const char* g_sql_create_pjsip_auth =
"create table pjsip_auth("

// basic info
"   object_type         varchar(255),"
"   object_name         varchar(1023),"
"   username            varchar(1023),"
"   endpoint_name       varchar(1023),"

// credential
"   auth_type           varchar(255),"
"   password            varchar(1023),"
"   md5_cred            varchar(1023),"

// etc
"   realm               varchar(255),"
"   nonce_lifetime      int,"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(object_name)"

");";

// voicemail_user
static const char* g_sql_drop_voicemail_user = "drop table if exists voicemail_user;";
static const char* g_sql_create_voicemail_user =
"create table voicemail_user("

// basic info
"   context varchar(255),"
"   mailbox varchar(255),"

// user info
"   full_name       varchar(1023),"
"   email           varchar(1023),"
"   pager           varchar(1023),"

// mail setting
"   server_email    varchar(1023),"
"   from_string     varchar(1023),"
"   mail_command    varchar(1023),"

"   language        varchar(255),"
"   timezone        varchar(255),"
"   callback        varchar(255),"
"   dialout         varchar(255),"
"   unique_id       varchar(255),"
"   exit_context    varchar(255),"

"   say_duration_minimum    int,"
"   say_envelope            varchar(255),"
"   say_cid                 varchar(255),"

"   attach_message          varchar(255),"
"   attachment_format      varchar(255),"
"   delete_message          varchar(255),"
"   volume_gain             real,"
"   can_review              varchar(255),"
"   call_operator           varchar(255),"

// message info
"   max_message_count       int,"
"   max_message_length      int,"
"   new_message_count       int,"
"   old_message_count       int,"

// imap info
"   imap_user               varchar(255),"
"   imap_server             varchar(255),"
"   imap_port               varchar(255),"
"   imap_flag               varchar(255),"

// timestamp. UTC."
"   tm_update         datetime(6),"   // update time."

"   primary key(context, mailbox)"
");";


#endif /* SRC_DB_SQL_CREATE_H_ */
