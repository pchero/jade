/*
 * zmq_handler.c
 *
 *  Created on: Dec 20, 2017
 *      Author: pchero
 */



#include <zmq.h>
#include <string.h>

#include "common.h"
#include "slog.h"

#include "zmq_handler.h"

#define DEF_LOCAL_ADDR "ipc:///tmp/jade_local.pub"

extern app* g_app;

void* g_zmq_contxt = NULL;    // zmq context for zmq_handler.
void* g_zmq_sock_pub = NULL;  // zmq socket for publish

static bool init_zmq_context(void);

static bool init_zmq_sock_pub(void);
static void* get_zmq_sock_pub(void);

static int s_sendmore(void* socket, const char* data);


/**
 * @brief Initiate zmq_hanler.
 */
bool zmq_init_handler(void)
{
  int ret;

  // initiate context
  ret = init_zmq_context();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate context.");
    return false;
  }

  // initiate publish socket
  ret = init_zmq_sock_pub();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate publish socket.");
    return false;
  }

  return true;
}

void zmq_term_handler(void)
{
  // close sockets
  // close pub
  if(g_zmq_sock_pub != NULL) {
    zmq_close(g_zmq_sock_pub);
  }
  g_zmq_sock_pub = NULL;

  // close context
  if(g_zmq_contxt != NULL) {
    zmq_ctx_destroy(g_zmq_contxt);
  }
  g_zmq_contxt = NULL;

  return;
}

/**
 * Initiate zmq context.
 * @return
 */
static bool init_zmq_context(void)
{
  // destroy it if it exists.
  if(g_zmq_contxt != NULL) {
    zmq_ctx_destroy(g_zmq_contxt);
  }

  // create new context
  g_zmq_contxt = zmq_ctx_new();
  if(g_zmq_contxt == NULL) {
    slog(LOG_ERR, "Could not initiate context for zmq.");
    return false;
  }

  return true;
}

/**
 * @brief initiate socket for publish and bind the socket.
 */
static bool init_zmq_sock_pub(void)
{
  int ret;
  const char* zmq_addr_pub;

  // close the socket if it exist.
  if(g_zmq_sock_pub != NULL) {
    zmq_close(g_zmq_sock_pub);
  }

  // create new socket
  g_zmq_sock_pub = zmq_socket(g_zmq_contxt, ZMQ_PUB);
  if(g_zmq_sock_pub == NULL) {
    slog(LOG_ERR, "Could not initiate socket for publish.");
    return false;
  }

  // get bind addr
  zmq_addr_pub = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "zmq_addr_pub"));
  if(zmq_addr_pub == NULL) {
    slog(LOG_ERR, "Could not get zmq_addr_pub info.");
    return false;
  }

  // bind the socket
  ret = zmq_bind(g_zmq_sock_pub, zmq_addr_pub);
  if(ret != 0) {
    slog(LOG_ERR, "Could not bind publish socket. err[%d:%s]", errno, strerror(errno));
    return false;
  }

  // bind local
  ret = zmq_bind(g_zmq_sock_pub, DEF_LOCAL_ADDR);
  if(ret != 0) {
    slog(LOG_ERR, "Could not bind publish socket. err[%d:%s]", errno, strerror(errno));
    return false;
  }

  return true;
}

static void* get_zmq_sock_pub(void)
{
  return g_zmq_sock_pub;
}

/**
 * @brief publish the given data to the given subscribers
 */
bool zmq_publish_message(const char* pub_target, json_t* j_data)
{
  char* tmp;
  int ret;
  void* sock;

  if((pub_target == NULL) || (j_data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get publish socket info
  sock = get_zmq_sock_pub();
  if(sock == NULL) {
    slog(LOG_ERR, "Could not get correct socket info.");
    return false;
  }

  // send publish target
  ret = s_sendmore(sock, pub_target);
  if(ret < 0) {
    slog(LOG_ERR, "Could not send target info.");
    return false;
  }

  // get data string
  tmp = json_dumps(j_data, JSON_ENCODE_ANY);
  if(tmp == NULL) {
    slog(LOG_ERR, "Could not dump the given data.");
    return false;
  }

  // send data string
  ret = zmq_send_string(sock, tmp);
  free(tmp);
  if(ret < 0) {
    slog(LOG_ERR, "Could not send the data.");
    return false;
  }

  return true;
}

/**
 * @brief String send.
 * Send given string to given zmq socket.
 * The only string data can be sent.
 * @return sent length
 */
int zmq_send_string(void* socket, const char* data)
{
  int size;

  if((socket == NULL) || (data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return -1;
  }

  size = zmq_send(socket, data, strlen(data), 0);

  return size;
}

/**
 @brief Receive zmq message.
 Receive 0MQ string from socket and convert into C string Caller must free
 returned string. Returns NULL if the context is being terminated.
 @param socket
 @return
 */
char* zmq_recv_string(void* socket)
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

/**
 * @brief Seting send more.
 * Sends string as 0MQ string, as multipart non-terminal
 * The only string data can be sent.
 * @return sent length
 */
static int s_sendmore(void* socket, const char* data)
{
  int size;

  if((socket == NULL) || (data == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return -1;
  }

  size = zmq_send(socket, data, strlen(data), ZMQ_SNDMORE);
  return size;
}

/**
 * Return zmq context
 */
void* zmq_get_context(void)
{
  return g_zmq_contxt;
}

/**
 * Return inproc address for pub
 * @return
 */
const char* zmq_get_pub_addr(void)
{
  return DEF_LOCAL_ADDR;
}
