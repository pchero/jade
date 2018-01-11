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

extern app* g_app;

void* g_zmq_contxt = NULL;    // zmq context for zmq_handler.
void* g_zmq_sock_pub = NULL;  // zmq socket for publish

static bool init_zmq_context(void);

static bool init_zmq_sock_pub(void);
static void* get_zmq_sock_pub(void);

static int s_send(void* socket, const char* data);
static int s_sendmore(void* socket, const char* data);


/**
 * @brief Initiate zmq_hanler.
 */
bool init_zmq_handler(void)
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

void term_zmq_handler(void)
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

  return true;
}

static void* get_zmq_sock_pub(void)
{
  return g_zmq_sock_pub;
}

/**
 * @brief publish the given data to the given subscribers
 */
bool publish_message(const char* pub_target, json_t* j_data)
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
  ret = s_send(sock, tmp);
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
static int s_send(void* socket, const char* data)
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
void* get_zmq_context(void)
{
  return g_zmq_contxt;
}
