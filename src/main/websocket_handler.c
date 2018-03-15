#define _GNU_SOURCE

#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <event.h>
#include <zmq.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <jansson.h>
#include <bsd/string.h>
#include <signal.h>

#include "bsd_queue.h"
#include "common.h"
#include "slog.h"
#include "utils.h"
#include "zmq_handler.h"

#define MAX_MSG_COUNT 1000

/**
 * Client session
 */
struct client_session {
  struct lws* wsi;    // websocket handler
  void* zmq_sock;     // zeromq socket for subscribe
  void* evt;          // event for socket

  char* addr;   ///< connected session address

  json_t* j_subs;   ///< subscription json array

  int recv_complete;
  char* recv_buf;

  int msg_count;  // message count
  TAILQ_HEAD(msg_head, msg_entry) msg_queue;
};

/**
 * Queue entry
 */
struct msg_entry {
  char* msg;
  TAILQ_ENTRY(msg_entry) entries;
};

enum protocols
{
  PROTOCOL_HTTP = 0,
  PROTOCOL_COUNT
};

static struct lws_protocols* g_protocols;



struct lws_context* g_websocket_context;
extern app* g_app;



static bool init_client_session(struct lws* wsi, struct client_session* session);
static void destroy_client_session(struct client_session* session);

static struct msg_entry* create_msg_entry(void);
static void destroy_msg_entry(struct msg_entry* entry);

static bool websocket_handler_established(struct lws *wsi, struct client_session* session, char* data, size_t len);
static bool websocket_handler_receive(struct client_session* session, char* data, size_t len);
static bool websocket_handler_server_writable(struct client_session* session, char* data, size_t len);
static bool websocket_handler_wsi_destroy(struct client_session* session, char* data, size_t len);

static bool send_session_message(struct client_session* session);
static bool recv_session_message_handler(struct client_session* session, json_t* j_msg);

static void zmq_sub_message_recv(int fd, short ev, void* arg);
static void add_subscription(struct client_session* session, const char* topic);
static void remove_subscription(struct client_session* session, const char* topic);

/**
 * HTTP weboscket handler
 * @param wsi
 * @param reason
 * @param user
 * @param in
 * @param len
 * @return
 */
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
  struct client_session* session;
  int ret;

  if(wsi == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 1;
  }

  session = (struct client_session*)user;

  switch(reason)
  {
    case LWS_CALLBACK_HTTP: {
      lws_return_http_status(wsi, 200, "OK");
    }
    break;

    case LWS_CALLBACK_ESTABLISHED: {
      slog(LOG_DEBUG, "Fired LWS_CALLBACK_ESTABLISHED.");
      ret = websocket_handler_established(wsi, session, in, len);
      if(ret == false) {
        slog(LOG_ERR, "Could not handle the event. LWS_CALLBACK_ESTABLISHED.");
      }
    }
    break;

    case LWS_CALLBACK_RECEIVE: {
      slog(LOG_DEBUG, "Fired LWS_CALLBACK_RECEIVE");
      ret = websocket_handler_receive(session, in, len);
      if(ret == false) {
        slog(LOG_ERR, "Could not handle the event. LWS_CALLBACK_RECEIVE.");
        // todo: close the session.
      }
    }
    break;

    case LWS_CALLBACK_SERVER_WRITEABLE: {
      slog(LOG_DEBUG, "Fired LWS_CALLBACK_SERVER_WRITEABLE.");
      ret = websocket_handler_server_writable(session, in, len);
      if(ret == false) {
        slog(LOG_ERR, "Could not handle the event. LWS_CALLBACK_SERVER_WRITEABLE.");
      }
    }
    break;

    case LWS_CALLBACK_WSI_DESTROY: {
      slog(LOG_DEBUG, "Fired LWS_CALLBACK_WSI_DESTROY.");
      ret = websocket_handler_wsi_destroy(session, in, len);
      if(ret == false) {
        slog(LOG_ERR, "Could not handle the event. LWS_CALLBACK_WSI_DESTROY.");
      }
    }
    break;

    default: {
      // do nothing
    }
    break;
  }

  return 0;
}

/**
 * Create client session
 */
static bool init_client_session(struct lws* wsi, struct client_session* session)
{
  char buf[1024];
  void* zmq_sock;
  int ret;
  struct event* evt;
  int fd;
  size_t length;

  const char* zmq_addr_pub;
  void* zmq_context;

  if((wsi == NULL) || (session == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired init_client_session.");

  // initiate
  session->recv_complete = 0;
  session->msg_count = 0;
  session->recv_buf = NULL;
  session->addr = NULL;
  session->j_subs = json_array();
  TAILQ_INIT(&(session->msg_queue));

  // set wsi
  session->wsi = wsi;

  // set addr
  lws_get_peer_simple(wsi, buf, sizeof(buf));
  slog(LOG_DEBUG, "Connected new client. addr[%s]", buf);
  session->addr = strdup(buf);

  // create new zmq sock
  zmq_context = get_zmq_context();
  zmq_addr_pub = get_zmq_pub_addr();
  slog(LOG_DEBUG, "Connecting to the local pub socket. addr[%s]", zmq_addr_pub);

  // connect zmq
  zmq_sock = zmq_socket(zmq_context, ZMQ_SUB);
  ret = zmq_connect(zmq_sock, zmq_addr_pub);
  if(ret != 0) {
    slog(LOG_ERR, "Could not connect to zmq socket. err[%d:%s]", errno, strerror(errno));
    destroy_client_session(session);
    return false;
  }
  session->zmq_sock= zmq_sock;

  // get file descriptor
  length = sizeof(fd);
  ret = zmq_getsockopt(session->zmq_sock, ZMQ_FD, &fd, &length);
  if(ret != 0) {
    slog(LOG_ERR, "Could not get zmq fd. err[%d:%s]", errno, strerror(errno));
    destroy_client_session(session);
    return false;
  }

  // create event
  evt = event_new(g_app->evt_base, fd, EV_PERSIST | EV_READ, zmq_sub_message_recv, session);
  if(evt == NULL) {
    slog(LOG_ERR, "Could not create event for zmq messge subscribe handler.");
    destroy_client_session(session);
    return false;
  }

  // register event
  ret = event_add(evt, NULL);
  if(ret != 0) {
    slog(LOG_ERR, "Could not register event.");
    destroy_client_session(session);
    return false;
  }
  session->evt = evt;

  return true;
}

/**
 * Destroy client session
 */
static void destroy_client_session(struct client_session* session)
{
  struct msg_entry* entry;
  struct msg_entry* entry_tmp;

  if(session == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  // delete all msg
  TAILQ_FOREACH_SAFE(entry, &(session->msg_queue), entries, entry_tmp) {
    TAILQ_REMOVE(&(session->msg_queue), session->msg_queue.tqh_first, entries);
    destroy_msg_entry(entry);
  }

  // free all members.
  sfree(session->addr);
  zmq_close(session->zmq_sock);
  if(session->evt != NULL) {
    event_del(session->evt);
    event_free(session->evt);
  }
  json_decref(session->j_subs);

  return;
}

/**
 * Create msg_entry
 */
static struct msg_entry* create_msg_entry(void)
{
  struct msg_entry* entry;

  entry = calloc(1, sizeof(struct msg_entry));
  entry->msg = NULL;

  return entry;
}

/**
 * Destroy msg_entry
 */
static void destroy_msg_entry(struct msg_entry* entry)
{
  if(entry == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  sfree(entry->msg);
  sfree(entry);
}

/**
 * Add the message to the given session.
 * It will be sent when the session is receivable.
 */
bool add_session_message(struct client_session* session, json_t* j_msg)
{
  struct msg_entry* entry;
  char* tmp;
  char* message;

  if((session == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  if(session->msg_count >= MAX_MSG_COUNT) {
    slog(LOG_WARNING, "The queue message size exceed maximum message count. msg_count[%d]", session->msg_count);
    return false;
  }

  // dump message
  tmp = json_dumps(j_msg, JSON_ENCODE_ANY);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not dump the message.");
    return false;
  }

  // create entry
  entry = create_msg_entry();

  // create message
  // Add the padding data(LWS_PRE) is important.
  // See detail (https://libwebsockets.org/lws-api-doc-master/html/group__sending-data.html)
  message = calloc(1, LWS_PRE + strlen(tmp) + 1);
  strcpy(message + LWS_PRE, tmp);
  sfree(tmp);
//  slog(LOG_DEBUG, "Copied message. msg[%s]", message + LWS_PRE);

  entry->msg = message;

  // insert entry
  TAILQ_INSERT_TAIL(&(session->msg_queue), entry, entries);
  session->msg_count++;

  return true;
}

/**
 * @brief Send queued message to the session.
 * This should be called when the session is ready to receive the message.
 * @return true: Succeed, false: Failed
 */
static bool send_session_message(struct client_session* session)
{
  struct msg_entry* entry;
  char* message;
  int ret;

  if(session == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get message
  entry = session->msg_queue.tqh_first;
  if(entry == NULL) {
    slog(LOG_DEBUG, "No more queue message left.");
    return true;
  }

  message = entry->msg;
  if(message == NULL) {
    slog(LOG_ERR, "Could not get correct message info.");

    // remove entry from the queue
    TAILQ_REMOVE(&(session->msg_queue), session->msg_queue.tqh_first, entries);
    destroy_msg_entry(entry);

    return false;
  }

  // send message
  // we send text message only.
  ret = lws_write(session->wsi, (unsigned char*)message + LWS_PRE, strlen(message + LWS_PRE), LWS_WRITE_TEXT);
  slog(LOG_DEBUG, "Sent message result. ret[%d]", ret);

  // remove entry from the queue
  TAILQ_REMOVE(&(session->msg_queue), session->msg_queue.tqh_first, entries);
  destroy_msg_entry(entry);

  session->msg_count--;

  return true;
}

static json_t* recv_zmq_msg(void* socket)
{
  json_t* j_tmp;
  json_t* j_res;
  char* topic;
  char* msg;
  int more;
  size_t more_size;

  if(socket == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // get topic
  topic = s_recv(socket);
  if(topic == NULL) {
    // no more message.
    return NULL;
  }

  more_size = sizeof(more);
  zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
  if(more != 1) {
    // something was wrong.
    slog(LOG_ERR, "Could not get message.");
    sfree(topic);
    return NULL;
  }

  // get message
  msg = s_recv(socket);

  // parse
  j_tmp = json_loads(msg, JSON_DECODE_ANY, NULL);
  free(msg);
  if(j_tmp == NULL) {
    slog(LOG_NOTICE, "Wrong message format.");
    sfree(topic);
    return NULL;
  }

  // set message
  j_res = json_object();
  json_object_set_new(j_res, topic, j_tmp);
  free(topic);

  return j_res;
}

/**
 * Receive received subscribe messages.
 * @param fd
 * @param ev
 * @param arg
 */
static void zmq_sub_message_recv(int fd, short ev, void* arg)
{
  struct client_session* session;
  uint32_t events;
  size_t len;
  int ret;
  json_t* j_data;

  if(arg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  session = (struct client_session*)arg;

  // get event
  events = 0;
  len = sizeof(events);
  ret = zmq_getsockopt(session->zmq_sock, ZMQ_EVENTS, &events, &len);
  if(ret == -1) {
    slog(LOG_ERR, "Could not get zmq event type.");
    return;
  }

  if(events & ZMQ_POLLIN) {
    slog(LOG_DEBUG, "Received zmq message.");

    while(1) {
      j_data = recv_zmq_msg(session->zmq_sock);
      if(j_data == NULL) {
        break;
      }

      // add message
      add_session_message(session, j_data);
      json_decref(j_data);
    }

    // request writable callback
    lws_callback_on_writable(session->wsi);
  }

  return;
}

static void add_subscription(struct client_session* session, const char* topic)
{
  int idx;
  json_t* j_tmp;
  const char* tmp_const;
  int ret;
  bool flg_exist;

  if((session == NULL) || (topic == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  // check exist
  flg_exist = false;
  json_array_foreach(session->j_subs, idx, j_tmp) {
    tmp_const = json_string_value(j_tmp);
    ret = strcmp(tmp_const, topic);
    if(ret == 0) {
      flg_exist = true;
      break;
    }
  }

  if(flg_exist == false) {
    json_array_append_new(session->j_subs, json_string(topic));
  }

  return;
}

static void remove_subscription(struct client_session* session, const char* topic)
{
  int idx;
  json_t* j_tmp;
  const char* tmp_const;
  int ret;

  if((session == NULL) || (topic == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  // remove item
  json_array_foreach(session->j_subs, idx, j_tmp) {
    tmp_const = json_string_value(j_tmp);
    ret = strcmp(tmp_const, topic);
    if(ret == 0) {
      json_array_remove(session->j_subs, idx);
      break;
    }
  }

  return;
}

/**
 * Websocket received message handler
 * @param state
 * @param msg
 * @return
 */
static bool recv_session_message_handler(struct client_session* session, json_t* j_msg)
{
  const char* type;
  const char* topic;
  char* tmp;
  int ret;

  if((session == NULL) || (j_msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired websocket_recv_message_handler. session_addr[%s]", session->addr);

  // get type
  type = json_string_value(json_object_get(j_msg, "type"));
  if(type == NULL) {
    slog(LOG_NOTICE, "Could not get type info.");
    json_decref(j_msg);
    return false;
  }

  // get topic
  topic = json_string_value(json_object_get(j_msg, "topic"));
  if(topic == NULL) {
    slog(LOG_NOTICE, "Could not get topic info.");
    json_decref(j_msg);
    return false;
  }
  slog(LOG_DEBUG, "Received message info. type[%s], topic[%s]", type, topic);

  // message parse
  if(strcmp(type, "subscribe") == 0) {
    ret = zmq_setsockopt(session->zmq_sock, ZMQ_SUBSCRIBE, topic, strlen(topic));
    if(ret != 0) {
      slog(LOG_ERR, "Could not add the subscription. session_addr[%s], topic[%s], err[%d:%s]",
          session->addr, topic, errno, strerror(errno));
      return false;
    }
    add_subscription(session, topic);
  }
  else if(strcmp(type, "unsubscribe") == 0) {
    ret = zmq_setsockopt(session->zmq_sock, ZMQ_UNSUBSCRIBE, topic, strlen(topic));
    if(ret != 0) {
      slog(LOG_ERR, "Could not unsubscribe topic. session_addr[%s], topic[%s], err[%d:%s]",
          session->addr, topic, errno, strerror(errno));
      return false;
    }
    remove_subscription(session, topic);
  }
  else {
    slog(LOG_ERR, "Wrong message type. tyep[%s]", type);
    return false;
  }

  tmp = json_dumps(session->j_subs, JSON_ENCODE_ANY);
  slog(LOG_INFO, "Updated websocket client subscription. client[%s], subs[%s]", session->addr, tmp);
  sfree(tmp);

  return 0;
}

/**
 * Websocket server event handler.
 * event: LWS_CALLBACK_WSI_DESTROY
 * @param session
 * @param data
 * @param len
 * @return
 */
static bool websocket_handler_wsi_destroy(struct client_session* session, char* data, size_t len)
{
  if(session == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  destroy_client_session(session);
  return true;
}


/**
 * Websocket server event handler.
 * event: LWS_CALLBACK_SERVER_WRITEABLE
 * @param session
 * @param data
 * @param len
 * @return
 */
static bool websocket_handler_server_writable(struct client_session* session, char* data, size_t len)
{
  int ret;

  if(session == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // send seesion message
  ret = send_session_message(session);
  if(ret == false) {
    slog(LOG_ERR, "Could not send message to the session.");
    return false;
  }

  // if there's session message remained, request writable callback.
  if(session->msg_count > 0) {
    lws_callback_on_writable(session->wsi);
  }

  return true;
}

/**
 * Websocket event handler.
 * event: LWS_CALLBACK_ESTABLISHED
 * @param wsi
 * @param session
 * @param data
 * @param len
 * @return
 */
static bool websocket_handler_established(struct lws *wsi, struct client_session* session, char* data, size_t len)
{
  int ret;

  if((wsi == NULL) || (session == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired websocket_handler_established.");

  ret = init_client_session(wsi, session);
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate client session info.");
    return false;
  }

  // make writable
  lws_callback_on_writable(session->wsi);

  return true;
}

/**
 * Websocket event handler
 * event: LWS_CALLBACK_RECEIVE
 * @param session
 * @param data
 * @param len
 * @return
 */
static bool websocket_handler_receive(struct client_session* session, char* data, size_t len)
{
  int ret;
  json_t* j_data;
  char* tmp;

  if(session == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_DEBUG, "Fired websocket_recv_handler.");

  // check binary
  // we allow text only.
  ret = lws_frame_is_binary(session->wsi);
  if(ret == true) {
    slog(LOG_NOTICE, "Received binary message. Just drop the message.");

    // just return true.
    // return false only when the socket could not receive the message.
    return true;
  }

  // check final
  session->recv_complete = lws_is_final_fragment(session->wsi);

  // copy
  asprintf(&tmp, "%s%s", session->recv_buf? : "", data);
  sfree(session->recv_buf);
  session->recv_buf = tmp;

  // if the message is not complete,
  // wait for next message
  if(session->recv_complete == false) {
    return true;
  }

  // load the message
  j_data = json_loads(session->recv_buf, JSON_DECODE_ANY, NULL);
  sfree(session->recv_buf);
  if(j_data == NULL) {
    slog(LOG_NOTICE, "Wrong type of message received. Could not parse the message.");

    // just return true.
    // return false only when the socket could not receive the message.
    return true;
  }

  ret = recv_session_message_handler(session, j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not handle the message in a correct way.");

    // just return true.
    // return false only when the socket could not receive the message.
    return true;
  }

  return true;
}

/**
 * Initiate websocket handler
 * @return
 */
bool init_websocket_handler(void)
{
  struct lws_context_creation_info info;
  const char* addr;
  const char* port;
  const char* pem_file;
  int ret;

  memset(&info, 0, sizeof(info));

  // get init info
  addr = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "websock_addr"));
  port = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "websock_port"));
  pem_file = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "https_pemfile"));
  slog(LOG_INFO, "Initiating websock. addr[%s], port[%s]", addr, port);

  // set protocols
  g_protocols = calloc(2, sizeof(struct lws_protocols));

  // http protocol
  g_protocols[0].name = "http-only";
  g_protocols[0].callback = callback_http;
  g_protocols[0].per_session_data_size = sizeof(struct client_session);
  g_protocols[0].rx_buffer_size = 0;
  g_protocols[0].tx_packet_size = 0;
  g_protocols[0].user = NULL;

  g_protocols[1].name = NULL;
  g_protocols[1].callback = NULL;
  g_protocols[1].per_session_data_size = 0;
  g_protocols[1].rx_buffer_size = 0;
  g_protocols[1].tx_packet_size = 0;
  g_protocols[1].user = NULL;

  // initiate options
  info.iface = addr;
  info.port = atoi(port);
  info.protocols = g_protocols;
  info.gid = -1;
  info.uid = -1;
  info.options = LWS_SERVER_OPTION_LIBEVENT;
  info.ssl_cert_filepath = pem_file;
  info.ssl_private_key_filepath = pem_file;
  info.options |= LWS_SERVER_OPTION_REDIRECT_HTTP_TO_HTTPS;
  lws_set_log_level(0, NULL);
  g_websocket_context = lws_create_context(&info);
  if(g_websocket_context == NULL) {
    slog(LOG_ERR, "Could not create lws context.");
    return false;
  }

  ret = lws_event_initloop(g_websocket_context, g_app->evt_base, 0);
  slog(LOG_DEBUG, "Check value. ret[%d]", ret);

  return true;
}

/**
 * Terminate websocket handler
 */
void term_websocket_handler(void)
{
  lws_context_destroy(g_websocket_context);
}

