/*
 * data_handler.c
 *
 *  Created on: Apr 2, 2017
 *      Author: pchero
 */


#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <jansson.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <fcntl.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "db_sql_create.h"
#include "event_handler.h"
#include "ami_handler.h"
#include "ami_event_handler.h"
#include "action_handler.h"


#define BUFLEN 20
#define MAX_AMI_RECV_BUF_LEN  409600

extern app* g_app;
extern struct event_base*  g_base;

static int g_ami_sock = 0;
static char g_ami_buffer[MAX_AMI_RECV_BUF_LEN];
struct event* g_ev_ami_handler = NULL;

static void cb_ami_message_receive_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_ami_connect(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_ami_connection_check(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);
static void cb_ami_status_check(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg);

static bool init_ami_database(void);
static bool init_ami_connect(void);
static bool send_init_actions(void);

static void free_ev_ami_handler(void);
static void update_ev_ami_handler(struct event* ev);

static bool ami_connect(void);
static bool ami_login(void);

static bool is_ev_ami_handler_running(void);

static void release_ami_connection(void);


/**
 * Callback function for ami message receive.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_ami_message_receive_handler(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  char buf[10];
  char* ami_msg;
  int ret;
  int len;

  ret = is_ev_ami_handler_running();
  if(ret == false) {
    return;
  }

  // receive
  while(1) {
    ret = recv(g_ami_sock, buf, 1, 0);
    if(ret != 1) {
      if(errno == EAGAIN) {
        return;
      }

      // something was wrong. update connected status
      slog(LOG_WARNING, "Could not receive correct message from the Asterisk. err[%d:%s]", errno, strerror(errno));
      bzero(g_ami_buffer, sizeof(g_ami_buffer));
      release_ami_connection();
      return;
    }

    len = strlen(g_ami_buffer);
    if(len >= sizeof(g_ami_buffer)) {
      slog(LOG_ERR, "Too much big data. Just clean up the buffer. size[%d]", len);
      bzero(g_ami_buffer, sizeof(g_ami_buffer));
      continue;
    }

    g_ami_buffer[len] = buf[0];
    if(len < 4) {
      // not ready to check the end of message.
      continue;
    }

    // check received all ami message
    len = strlen(g_ami_buffer);
    if((g_ami_buffer[len-4] == '\r')
        && (g_ami_buffer[len-3] == '\n')
        && (g_ami_buffer[len-2] == '\r')
        && (g_ami_buffer[len-1] == '\n')
        ) {
      ami_msg = strdup(g_ami_buffer);
      bzero(g_ami_buffer, sizeof(g_ami_buffer));

      ami_message_handler(ami_msg);
      sfree(ami_msg);
    }
  }

  return;
}

/**
 * Callback function for asterisk ami connect.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_ami_connect(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  int ret;
  struct event* ev;

  ret = is_ev_ami_handler_running();
  if(ret == true) {
    return;
  }
  slog(LOG_NOTICE, "Fired cb_ami_connect. connected[%d]", ret);

  // ami connect
  ret = ami_connect();
  if(ret == false) {
    slog(LOG_ERR, "Could not connect to asterisk ami.");
    return;
  }

  // add ami event handler
  ev = event_new(g_base, g_ami_sock, EV_READ | EV_PERSIST, cb_ami_message_receive_handler, NULL);
  event_add(ev, NULL);

  // update event ami handler
  update_ev_ami_handler(ev);

  return;
}

/**
 * Callback function for check the ami/asterisk status info.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_ami_status_check(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_tmp;
  json_t* j_data;
  char* action_id;
  char* sql;
  int ret;

  slog(LOG_DEBUG, "Fired cb_ami_status_check.");

  // create initial info if need.
  asprintf(&sql, "insert or ignore into system(id) values(1)");
  db_ctx_exec(g_db_ast, sql);
  sfree(sql);

  //// CoreStatus
  // create data
  j_data = json_pack("{s:s}",
      "id", "1"
      );

  // create action
  action_id = gen_uuid();
  j_tmp = json_pack("{s:s, s:s}",
      "Action",   "CoreStatus",
      "ActionID", action_id
      );
  ret = send_ami_cmd(j_tmp);
  sfree(action_id);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami request.");
    json_decref(j_tmp);
    json_decref(j_data);
    return;
  }

  // insert action
  insert_action(json_string_value(json_object_get(j_tmp, "ActionID")), "corestatus", j_data);
  json_decref(j_tmp);
  json_decref(j_data);

  //// CoreSettings
  // create data
  j_data = json_pack("{s:s}",
      "id", "1"
      );

  // create action
  action_id = gen_uuid();
  j_tmp = json_pack("{s:s, s:s}",
      "Action",     "CoreSettings",
      "ActionID",   action_id
      );
  ret = send_ami_cmd(j_tmp);
  sfree(action_id);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami request.");
    json_decref(j_tmp);
    json_decref(j_data);
    return;
  }

  // insert action
  insert_action(json_string_value(json_object_get(j_tmp, "ActionID")), "coresettings", j_data);
  json_decref(j_tmp);
  json_decref(j_data);

  return;
}

/**
 * Check the ami connection info.
 * @param fd
 * @param event
 * @param arg
 */
static void cb_ami_connection_check(__attribute__((unused)) int fd, __attribute__((unused)) short event, __attribute__((unused)) void *arg)
{
  json_t* j_tmp;
  int ret;

  slog(LOG_DEBUG, "Fired cb_ami_connection_check.");

  j_tmp = json_pack("{s:s}",
      "Action", "Ping"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ping request. ret[%d]", ret);
    return;
  }

  return;
}

/**
 * Terminate ami handler.
 * @return
 */
void term_ami_handler(void)
{
  slog(LOG_DEBUG, "Fired term_ami_handler.");
  release_ami_connection();
  return;
}

/**
 * Asterisk ami login.
 * @return
 */
static bool ami_login(void)
{
  int ret;
  const char* username;
  const char* password;
  json_t* j_tmp;

  if(g_app == NULL) {
    return false;
  }
  slog(LOG_DEBUG, "Fired ami_login");

  username = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_username"));
  password = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_password"));

  j_tmp = json_pack("{s:s, s:s, s:s}",
      "Action",   "Login",
      "Username", username,
      "Secret",   password
      );

  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not login.");
    return false;
  }
  slog(LOG_NOTICE, "The ami login result. res[%d]", ret);

  return true;
}

/**
 * AMI connect and send initial Action commands.
 * @return
 */
static bool ami_connect(void)
{
  int ret;

  // init ami connect
  ret = init_ami_connect();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ami connection.");
    return false;
  }

  // init ami database
  ret = init_ami_database();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ami database.");
    return false;
  }

  // login
  ret = ami_login();
  if(ret == false) {
    slog(LOG_ERR, "Could not login.");
    return false;
  }

  // bad idea
  // but after login to asterisk, we need to wait for second before sending a command.
  sleep(1);

  // send get all initial ami request
  ret = send_init_actions();
  if(ret == false) {
    slog(LOG_ERR, "Could not send init info.");
    return false;
  }

  return true;
}

/**
 * Initialize ami database.
 * @return Success: true\n
 * Failure: false
 */
static bool init_ami_database(void)
{
  int ret;

  // action
  db_ctx_exec(g_db_ast, g_sql_drop_action);
  ret = db_ctx_exec(g_db_ast, g_sql_create_action);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "action");
    return false;
  }

  // system
  db_ctx_exec(g_db_ast, g_sql_drop_system);
  ret = db_ctx_exec(g_db_ast, g_sql_create_system);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "system");
    return false;
  }

  // channel
  db_ctx_exec(g_db_ast, g_sql_drop_channel);
  ret = db_ctx_exec(g_db_ast, g_sql_create_channel);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "channel");
    return false;
  }

  // peer
  db_ctx_exec(g_db_ast, g_sql_drop_peer);
  ret = db_ctx_exec(g_db_ast, g_sql_create_peer);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "peer");
    return false;
  }

  // queue_param
  db_ctx_exec(g_db_ast, g_sql_drop_queue_param);
  ret = db_ctx_exec(g_db_ast, g_sql_create_queue_param);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_param");
    return false;
  }

  // queue_member
  db_ctx_exec(g_db_ast, g_sql_drop_queue_member);
  ret = db_ctx_exec(g_db_ast, g_sql_create_queue_member);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_member");
    return false;
  }

  // queue_member
  db_ctx_exec(g_db_ast, g_sql_drop_queue_entry);
  ret = db_ctx_exec(g_db_ast, g_sql_create_queue_entry);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "queue_entry");
    return false;
  }

  // database
  db_ctx_exec(g_db_ast, g_sql_drop_database);
  ret = db_ctx_exec(g_db_ast, g_sql_create_database);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "database");
    return false;
  }

  // registry
  db_ctx_exec(g_db_ast, g_sql_drop_registry);
  ret = db_ctx_exec(g_db_ast, g_sql_create_registry);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "registry");
    return false;
  }

  // agent
  db_ctx_exec(g_db_ast, g_sql_drop_agent);
  ret = db_ctx_exec(g_db_ast, g_sql_create_agent);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "agent");
    return false;
  }

  // device_state
  db_ctx_exec(g_db_ast, g_sql_drop_device_state);
  ret = db_ctx_exec(g_db_ast, g_sql_create_device_state);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "device_state");
    return false;
  }

  // parking_lot
  db_ctx_exec(g_db_ast, g_sql_drop_parking_lot);
  ret = db_ctx_exec(g_db_ast, g_sql_create_parking_lot);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "parking_lot");
    return false;
  }

  // parked_call
  db_ctx_exec(g_db_ast, g_sql_drop_parked_call);
  ret = db_ctx_exec(g_db_ast, g_sql_create_parked_call);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "parked_call");
    return false;
  }

  // pjsip_contact_status
  db_ctx_exec(g_db_ast, g_sql_drop_pjsip_contact_status);
  ret = db_ctx_exec(g_db_ast, g_sql_create_pjsip_contact_status);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "pjsip_contact_status");
    return false;
  }

  // pjsip_endpoint
  db_ctx_exec(g_db_ast, g_sql_drop_pjsip_endpoint);
  ret = db_ctx_exec(g_db_ast, g_sql_create_pjsip_endpoint);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "pjsip_endpoint");
    return false;
  }

  // pjsip_aor
  db_ctx_exec(g_db_ast, g_sql_drop_pjsip_aor);
  ret = db_ctx_exec(g_db_ast, g_sql_create_pjsip_aor);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "pjsip_aor");
    return false;
  }

  // pjsip_auth
  db_ctx_exec(g_db_ast, g_sql_drop_pjsip_auth);
  ret = db_ctx_exec(g_db_ast, g_sql_create_pjsip_auth);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "pjsip_auth");
    return false;
  }

  // voicemail_user
  db_ctx_exec(g_db_ast, g_sql_drop_voicemail_user);
  ret = db_ctx_exec(g_db_ast, g_sql_create_voicemail_user);
  if(ret == false) {
    slog(LOG_ERR, "Could not create table. table[%s]", "voicemail_user");
    return false;
  }

  return true;
}

static bool send_init_actions(void)
{
  json_t* j_tmp;
  int ret;

  // sip peers
  j_tmp = json_pack("{s:s}",
      "Action", "SipPeers"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "SipPeers");
    return false;
  }

  // queue status
  j_tmp = json_pack("{s:s}",
      "Action", "QueueStatus"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "QueueStatus");
    return false;
  }

//  // database
//  action_id = gen_uuid();
//  j_tmp = json_pack("{s:s, s:s, s:s}",
//      "Action",   "Command",
//      "Command",  "database show",
//      "ActionID", action_id
//      );
//  sfree(action_id);
//  ret = send_ami_cmd(j_tmp);
//  if(ret == false) {
//    json_decref(j_tmp);
//    slog(LOG_ERR, "Could not send ami action. action[%s]", "Command");
//    return false;
//  }
//  insert_action(json_string_value(json_object_get(j_tmp, "ActionID")), "command.databaseshowall");
//  json_decref(j_tmp);

  // registry
  j_tmp = json_pack("{s:s}",
      "Action", "SIPshowregistry"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "SIPshowregistry");
    return false;
  }

  // agents
  j_tmp = json_pack("{s:s}",
      "Action", "Agents"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "Agents");
    return false;
  }

  // parking_lot
  j_tmp = json_pack("{s:s}",
      "Action", "ParkingLots"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "Parkinglosts");
    return false;
  }

  // parked_call
  j_tmp = json_pack("{s:s}",
      "Action", "ParkedCalls"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "ParkedCalls");
    return false;
  }

  // device state
  j_tmp = json_pack("{s:s}",
      "Action", "DeviceStateList"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "DeviceStateList");
    return false;
  }

  // voicemail_user
  j_tmp = json_pack("{s:s}",
      "Action", "VoicemailUsersList"
      );
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action. action[%s]", "VoicemailUsersList");
    return false;
  }

  return true;
}

/**
 * Initiate ami_handler.
 * @return
 */
bool init_data_handler(void)
{
  struct timeval tm_event;
  struct event* ev;

  tm_event.tv_sec = 1;
  tm_event.tv_usec = 0;

  // ami connect
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_ami_connect, NULL);
  event_add(ev, &tm_event);
  add_event_handler(ev);

  // check ami connection
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_ami_connection_check, NULL);
  event_add(ev, &tm_event);
  add_event_handler(ev);

  // check ami status
  ev = event_new(g_base, -1, EV_TIMEOUT | EV_PERSIST, cb_ami_status_check, NULL);
  event_add(ev, &tm_event);
  add_event_handler(ev);

  return true;
}

/**
 * Return the event ami handler is running.
 * @return
 */
static bool is_ev_ami_handler_running(void)
{
  if(g_ev_ami_handler == NULL) {
    return false;
  }

  return true;
}

/**
 * Release ami connection info.
 */
static void release_ami_connection(void)
{
  slog(LOG_NOTICE, "Fired release_ami_connection.");

  free_ev_ami_handler();
  if(g_ami_sock != -1) {
    close(g_ami_sock);
  }
  g_ami_sock = -1;
  return;
}

/**
 * Free the event ami handler.
 */
static void free_ev_ami_handler(void)
{
  int ret;

  slog(LOG_NOTICE, "Fired free_ev_ami_handler.");

  ret = is_ev_ami_handler_running();
  if(ret == false) {
    return;
  }

  // free
  event_free(g_ev_ami_handler);
  g_ev_ami_handler = NULL;

  return;
}

/**
 * update ev_ami_handler.
 * @param ev
 */
static void update_ev_ami_handler(struct event* ev)
{
  if(ev == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired update_ev_ami_handler.");

  free_ev_ami_handler();

  g_ev_ami_handler = ev;
}

/**
 * Init ami connect.
 * Create socket and connect to ami.
 * @return Success: true\n
 * Failure: false
 */
static bool init_ami_connect(void)
{
  const char* serv_addr;
  const char* serv_port;
  int port;
  struct sockaddr_in server;
  int ret;
  int flag;

  serv_addr = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_serv_addr"));
  serv_port = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "ami_serv_port"));
  if((serv_addr == NULL) || (serv_port == NULL)) {
    return false;
  }
  port = atoi(serv_port);
  slog(LOG_INFO, "Connecting to the Asterisk. addr[%s], port[%d]", serv_addr, port);

  if(g_ami_sock != -1) {
    close(g_ami_sock);
  }

  // create socket
  g_ami_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(g_ami_sock == -1) {
    printf("Could not create socket.\n");
    return false;
  }
  slog(LOG_DEBUG, "Created socket to Asterisk.");

  // get server info
  server.sin_addr.s_addr = inet_addr(serv_addr);
  server.sin_family = AF_INET;
  server.sin_port = htons(port);

  //Connect to remote server
  bzero(g_ami_buffer, sizeof(g_ami_buffer));
  ret = connect(g_ami_sock , (struct sockaddr *)&server, sizeof(server));
  if(ret < 0) {
    slog(LOG_WARNING, "Could not connect to the Asterisk. err[%d:%s]", errno, strerror(errno));
    return false;
  }
  slog(LOG_DEBUG, "Connected to Asterisk.");

  // get/set socket option
  flag = fcntl(g_ami_sock, F_GETFL, 0);
  flag = flag|O_NONBLOCK;
  ret = fcntl(g_ami_sock, F_SETFL, flag);
  slog(LOG_DEBUG, "Set the non-block option for the Asterisk socket. ret[%d]", ret);

  // set ami_handler ami_sock
  set_ami_socket(g_ami_sock);

  return true;
}
