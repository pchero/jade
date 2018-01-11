
#include <stdio.h>
#include <stdlib.h>
#include <websock/websock.h>
#include <zmq.h>
#include <jansson.h>
#include <stdbool.h>

#include "common.h"
#include "slog.h"
#include "utils.h"
#include "zmq_handler.h"
#include "websock_handler.h"

struct websock_session {
  libwebsock_client_state* state;   // websock state
  void* zmq_sock;                   // zeromq socket for subscribe
  void* evt;                        // event for socket
};

extern app* g_app;


static void destroy_websock_session(struct websock_session* session);

static int onopen(libwebsock_client_state* state);
static int onmessage(libwebsock_client_state *state, libwebsock_message *msg);
static int onclose(libwebsock_client_state *state);

static void zmq_sub_message_recv(int fd, short ev, void* arg);
static json_t* recv_zmq_msg(void* socket);
static char* s_recv (void *socket);

/**
 *
 * @param session
 */
static void destroy_websock_session(struct websock_session* session)
{
  if(session == NULL) {
    return;
  }

  if(session->zmq_sock != NULL) {
    zmq_close(session->zmq_sock);
  }

  if(session->evt != NULL) {
    event_del(session->evt);
    event_free(session->evt);
  }

  sfree(session);

  return;
}

/**
 * message receive callback
 * @param state
 * @param msg
 * @return
 */
static int onmessage(libwebsock_client_state *state, libwebsock_message *msg)
{
  json_t* j_msg;
  const char* type;
  const char* topic;
  struct websock_session* session;
  int ret;

  if((state == NULL) || (msg == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 0;
  }
  slog(LOG_DEBUG, "Fired onmessage.");

  // allows only opcode==1(string)
  if(msg->opcode != 1) {
    slog(LOG_NOTICE, "Wrong opcode. opcode[%d]", msg->opcode);
    return 0;
  }

  // message load
  j_msg = json_loadb(msg->payload, msg->payload_len, JSON_DECODE_ANY, NULL);
  if(j_msg == NULL) {
    slog(LOG_NOTICE, "Could not load json info.");
    return 0;
  }

  // get type
  type = json_string_value(json_object_get(j_msg, "type"));
  if(type == NULL) {
    slog(LOG_NOTICE, "Could not get type info.");
    json_decref(j_msg);
    return 0;
  }

  // get topic
  topic = json_string_value(json_object_get(j_msg, "topic"));
  if(topic == NULL) {
    slog(LOG_NOTICE, "Could not get topic info.");
    json_decref(j_msg);
    return 0;
  }

  session = state->data;
  if(session == NULL) {
    slog(LOG_WARNING, "Could not get session info.");
    return 0;
  }

  // message parse
  if(strcmp(type, "subscribe") == 0) {
    ret = zmq_setsockopt(session->zmq_sock, ZMQ_SUBSCRIBE, topic, strlen(topic));
    if(ret != 0) {
      slog(LOG_ERR, "Could not subscribe topic. topic[%s], err[%d:%s]", topic, errno, strerror(errno));
    }
  }
  else if(strcmp(type, "unsubscribe") == 0) {
    ret = zmq_setsockopt(session->zmq_sock, ZMQ_UNSUBSCRIBE, topic, strlen(topic));
    if(ret != 0) {
      slog(LOG_ERR, "Could not unsubscribe topic. topic[%s], err[%d:%s]", topic, errno, strerror(errno));
    }
  }
  else {
    slog(LOG_ERR, "Wrong message type. tyep[%s]", type);
  }
  json_decref(j_msg);

  return 0;
}

/**
 * callback for when websocket connected
 * @param state
 * @return
 */
static int onopen(libwebsock_client_state* state)
{
  void* zmq_sock;
  int ret;
  struct websock_session* session;
  size_t length;
  int fd;
  struct event* evt;
  void* zmq_context;
  const char* addr;

  if(state == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 0;
  }
  slog(LOG_DEBUG, "Fired onopen");

  // create session info
  session = calloc(1, sizeof(struct websock_session));
  session->state = state;

  // get zmq info
  zmq_context = get_zmq_context();
  addr = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "zmq_addr_pub"));
  if((zmq_context == NULL) || (addr == NULL)) {
    slog(LOG_ERR, "Could not get zmq info.");
    return 1;
  }

  // create and connect zmq.
  zmq_sock = zmq_socket(zmq_context, ZMQ_SUB);
  ret = zmq_connect(zmq_sock, "tcp://localhost:8082");
  if(ret != 0) {
    slog(LOG_ERR, "Could not connect to zmq socket. err[%d:%s]", errno, strerror(errno));
    // destroy
    destroy_websock_session(session);
    return 1;
  }
  session->zmq_sock = zmq_sock;

  // get file descriptor
  length = sizeof(fd);
  ret = zmq_getsockopt(session->zmq_sock, ZMQ_FD, &fd, &length);
  if(ret != 0) {
    slog(LOG_ERR, "Could not get zmq fd. err[%d:%s]", errno, strerror(errno));
    destroy_websock_session(session);
    return 1;
  }

  // create event
  evt = event_new(g_app->evt_base, fd, EV_PERSIST | EV_READ, zmq_sub_message_recv, session);
  if(evt == NULL) {
    slog(LOG_ERR, "Could not create event for zmq_sub. err[%d:%s]", errno, strerror(errno));
    destroy_websock_session(session);
    return 1;
  }

  // register event
  session->evt = evt;
  ret = event_add(session->evt, NULL);
  if(ret != 0) {
    slog(LOG_ERR, "Could not register the event.");
    destroy_websock_session(session);
    return 1;
  }

  // add data
  state->data = session;

  return 0;
}

/**
 * Fired when the websocket closed.
 */
static int onclose(libwebsock_client_state *state)
{
  if(state == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return 1;
  }
  slog(LOG_DEBUG, "Fired onclose.");

  destroy_websock_session(state->data);

  libwebsock_close(state);

  return 0;
}

/**
 * Receive enveloped message from the zmqsock.
 */
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

static void zmq_sub_message_recv(int fd, short ev, void* arg)
{
  struct websock_session* session;
  uint32_t events;
  size_t len;
  int ret;
  json_t* j_data;
  char* tmp;

  if(arg == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
//  slog(LOG_DEBUG, "Fired zmq_sub_message_recv.");

  session = arg;

  // get event
  events = 0;
  len = sizeof(events);
  ret = zmq_getsockopt(session->zmq_sock, ZMQ_EVENTS, &events, &len);
  if(ret == -1) {
    slog(LOG_ERR, "Could not get zmq event type.");
    return;
  }

  // receive and send
  if(events & ZMQ_POLLIN) {
    slog(LOG_DEBUG, "Received zmq message.");

    while(1) {
      j_data = recv_zmq_msg(session->zmq_sock);
      if(j_data == NULL) {
        break;
      }

      // get send string
      tmp = json_dumps(j_data, JSON_ENCODE_ANY);
      json_decref(j_data);

      // send message
      ret = libwebsock_send_text(session->state, tmp);
      sfree(tmp);
    }
  }

  return;
}

/**
 * Receive zmq message.
 * @param socket
 * @return
 */
static char* s_recv (void *socket)
{
  char buffer[40960];
  int size;
  char* res;

  if(socket == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  size = zmq_recv(socket, buffer, sizeof(buffer), ZMQ_NOBLOCK);
  if(size == -1) {
    slog(LOG_ERR, "Could not receive zmq message.");
    return NULL;
  }
  else if(size > sizeof(buffer)) {
    slog(LOG_WARNING, "Over sized message received. len[%d]", size);
    size = sizeof(buffer);
  }

  buffer[size] = 0;
  res = strdup(buffer);

  return res;
}

bool init_websock_handler(void)
{
  libwebsock_context* ctx;
  int ret;
  const char* addr;
  const char* port;

  // need this.
  ret = evthread_use_pthreads();
  if(ret != 0) {
    slog(LOG_ERR, "Could not initiate libevent pthread.");
    return false;
  }

  // initiate libwebsock.
  ctx = libwebsock_init_base(g_app->evt_base, 0);
  if(ctx == NULL) {
    slog(LOG_ERR, "Could not initiate libwebsock.");
    return false;
  }

  // get addr info
  addr = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "websock_addr"));
  port = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "websock_port"));
  slog(LOG_INFO, "Initiating websock. addr[%s], port[%s]", addr, port);

  // listen
  libwebsock_bind(ctx, addr, port);

  // register callbacks
  ctx->onmessage = onmessage;
  ctx->onopen = onopen;
  ctx->onclose = onclose;

  return true;
}
