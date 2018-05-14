/*
 * db_sql_create.h
 *
 *  Created on: Oct 13, 2016
 *      Author: pchero
 */

#ifndef SRC_DB_SQL_CREATE_H_
#define SRC_DB_SQL_CREATE_H_



#define DEF_DB_TABLE_SIP_PEER           "sip_peer"
#define DEF_DB_TABLE_SIP_PEERACCOUNT    "sip_peeraccount"

#define DEF_DB_TABLE_USER_USERINFO      "user_userinfo"

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


// core_agi
static const char* g_sql_drop_core_agi = "drop table if exists core_agi;";
static const char* g_sql_create_core_agi =
"create table core_agi("

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

// etc
"   env         text,"
"   cmd         text,"

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

// info
"   size      int,"
"   load      varchar(255),"
"   version   varchar(255),"

// timestamp. UTC."
"   tm_update     datetime(6),"

"   primary key(name)"
");";


//// sip_peer
//static const char* g_sql_drop_sip_peer = "drop table if exists " DEF_DB_TABLE_SIP_PEER ";";
//static const char* g_sql_create_sip_peer =
//"create table " DEF_DB_TABLE_SIP_PEER " ("
//
//// identity
//"   peer         varchar(255)    not null,"
//"   status       varchar(255),"
//"   address      varchar(255),"     // ip_address:port
//
//// peer info
//"   channel_type     varchar(255),"   // channel type(SIP, ...)
//"   chan_object_type varchar(255),"   // (peer, ...)
//"   monitor_status   varchar(255),"
//
//"   dynamic          varchar(255),"   // dynamic(yes, ...)
//"   auto_force_port  varchar(255),"   //
//"   force_port       varchar(255),"   //
//"   auto_comedia     varchar(255),"   //
//"   comedia          varchar(255),"   //
//"   video_support    varchar(255),"   //
//"   text_support     varchar(255),"   //
//
//"   acl              varchar(255),"   //
//"   realtime_device  varchar(255),"   //
//"   description      varchar(255),"   //
//
//// timestamp. UTC."
//"   tm_update        datetime(6),"   // update time."
//
//"   primary key(peer)"
//
//");";

//// sip_peeraccount
//static const char* g_sql_drop_sip_peeraccount = "drop table if exists " DEF_DB_TABLE_SIP_PEERACCOUNT ";";
//static const char* g_sql_create_sip_peeraccount =
//"create table " DEF_DB_TABLE_SIP_PEERACCOUNT " ("
//
//// identity
//"   peer         varchar(255)    not null,"
//"   secret       varchar(255),"
//
//// info
//"   host       varchar(255),"
//"   context    varchar(255),"
//"   type       varchar(255)"
//
//// timestamp. UTC."
//"   tm_update        datetime(6),"   // update time."
//
//"   primary key(peer)"
//
//");";



//// queue param
//static const char* g_sql_drop_queue_param = "drop table if exists queue_param;";
//static const char* g_sql_create_queue_param =
//"create table queue_param("
//
//// identity
//"   name             varchar(255),"    // queue name.
//
//// status
//"   max              int,"             // max available calls in the queue.
//"   strategy         varchar(255),"    // queue strategy.
//"   calls            int,"             // waiting call count.
//"   hold_time        int,"             // average waiting time.
//"   talk_time        int,"             // average talk time.
//"   completed        int,"             // distributed call count.
//"   abandoned        int,"             // could not distributed call count.
//
//// performance
//"   service_level       int,"                 // service level interval time sec.
//"   service_level_perf  real  default 0.0,"   // service level performance(%). completed_call / (abandoned_call + completed_call) * 100
//
//// etc
//"   weight          int,"    // queue priority.
//
//// timestamp. UTC."
//"   tm_update       datetime(6),"   // update time."
//
//"   primary key(name)"
//
//");";


//// queue member
//static const char* g_sql_drop_queue_member = "drop table if exists queue_member;";
//static const char* g_sql_create_queue_member =
//"create table queue_member("
//
//// identity
//"   id             varchar(255),"   // member id(name@queue)
//"   queue_name     varchar(255),"   // queue name
//"   name           varchar(255),"   // member name
//
//"   location          varchar(255),"          // location
//"   state_interface   varchar(255),"          // state interface
//"   membership        varchar(255),"  // membership
//"   penalty           int,"           // penalty
//"   calls_taken       int,"           // call taken count.
//"   last_call         int,"
//"   last_pause        int,"
//"   in_call           int,"
//"   status            int,"
//"   paused            int,"
//"   paused_reason     text,"
//"   ring_inuse        int,"
//
//// timestamp. UTC."
//"   tm_update         datetime(6),"   // update time."
//
//"   primary key(queue_name, name)"
//
//");";


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
"   stop_space  varchar(255),"

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


// voicemail_user
static const char* g_sql_drop_voicemail_user = "drop table if exists voicemail_user;";
static const char* g_sql_create_voicemail_user =
"create table voicemail_user("

// basic info
"   id      varchar(255),"
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

"   primary key(id)"
");";

#endif /* SRC_DB_SQL_CREATE_H_ */
