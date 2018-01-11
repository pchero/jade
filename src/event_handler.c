/*
 * event_handler.c
 *
 *  Created on: Feb 5, 2017
 *      Author: pchero
 */


#include <event.h>
#include <signal.h>
#include <stdbool.h>
#include <errno.h>
#include <bsd/libutil.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "slog.h"
#include "event_handler.h"
#include "data_handler.h"
#include "config.h"

#define DEF_PID_FILEPATH "/var/run/jade_backend.pid"
#define DEF_PID_TIMEOUT_SEC 5
#define DEF_MAX_EVENT_COUNT 128

extern app* g_app;
struct event* g_ev_ast[DEF_MAX_EVENT_COUNT] = { NULL };  ///< asterisk events

static void cb_signal_term(int sock, short which, void* arg);
static void cb_signal_int(evutil_socket_t sig, short events, void *user_data);
static void cb_signal_pipe(evutil_socket_t sig, short events, void *user_data);
static void cb_signal_hup(evutil_socket_t sig, short events, void *user_data);

static void cb_pid_update(int sock, short which, void* arg);

static bool pid_update(void);


bool init_event_handler(void)
{
  struct event* ev;
  struct event* ev_pid;
  struct timeval tv;
  int ret;

  // initiate event base
  g_app->evt_base = event_base_new();
  if(g_app->evt_base == NULL) {
    slog(LOG_ERR, "Could not initiate event_base.");
    return false;
  }

  //// add signal handler
  // sigterm
  ev = evsignal_new(g_app->evt_base, SIGTERM, cb_signal_term, NULL);
  if(ev == NULL) {
    slog(LOG_ERR, "Could not create event for the signal handler. signal[%s]", "SIGTERM");
    return false;
  }
  ret = event_add(ev, NULL);
  if(ret != 0) {
    slog(LOG_ERR, "Could not add the signal handler. signal[%s]", "SIGTERM");
    return false;
  }
  add_event_handler(ev);

  // sigint
  ev = evsignal_new(g_app->evt_base, SIGINT, cb_signal_int, NULL);
  if(ev == NULL) {
    slog(LOG_ERR, "Could not create event for the signal handler. signal[%s]", "SIGINT");
    return false;
  }
  ret = event_add(ev, NULL);
  if(ret != 0) {
    slog(LOG_ERR, "Could not add the signal handler. signal[%s]", "SIGINT");
    return false;
  }
  add_event_handler(ev);

  // sigpipe
  ev = evsignal_new(g_app->evt_base, SIGPIPE, cb_signal_pipe, NULL);
  if(ev == NULL) {
    slog(LOG_ERR, "Could not create event for the signal handler. signal[%s]", "SIGPIPE");
    return false;
  }
  ret = event_add(ev, NULL);
  if(ret != 0) {
    slog(LOG_ERR, "Could not add the signal handler. signal[%s]", "SIGPIPE");
    return false;
  }
  add_event_handler(ev);

  // sighup
  ev = evsignal_new(g_app->evt_base, SIGHUP, cb_signal_hup, NULL);
  if(ev == NULL) {
    slog(LOG_ERR, "Could not create event for the signal handler. signal[%s]", "SIGHUP");
    return false;
  }
  ret = event_add(ev, NULL);
  if(ret != 0) {
    slog(LOG_ERR, "Could not add the signal handler. signal[%s]", "SIGHUP");
    return false;
  }
  add_event_handler(ev);

  // add pid update
  tv.tv_sec = DEF_PID_TIMEOUT_SEC;
  tv.tv_usec = 0;
  ev_pid = calloc(sizeof(struct event), 1);
  event_assign(ev_pid, g_app->evt_base, -1, 0, cb_pid_update, ev_pid);
  event_add(ev_pid, &tv);
  add_event_handler(ev_pid);

  return true;
}

void term_event_handler(void)
{
  int idx;

  slog(LOG_NOTICE, "Fired term_event_handler.");

  for(idx = 0; idx < DEF_MAX_EVENT_COUNT; idx++) {
    if(g_ev_ast[idx] == NULL) {
      continue;
    }

    event_free(g_ev_ast[idx]);
    g_ev_ast[idx] = NULL;
  }
}

/**
 * Signal handler.
 * signal: sig_term
 * @param sock
 * @param which
 * @param arg
 */
static void cb_signal_term(int sock, short which, void* arg)
{
  slog(LOG_INFO, "Fired cb_signal_term.");
  event_base_loopbreak(g_app->evt_base);

  return;
}

/**
 * Signal handler.
 * signal: sig_int
 * @param sock
 * @param which
 * @param arg
 */
static void cb_signal_int(evutil_socket_t sig, short events, void *user_data)
{
  slog(LOG_INFO, "Fired cb_signal_int.");
  event_base_loopbreak(g_app->evt_base);

  return;
}

/**
 * Signal handler.
 * signal: sig_pipe
 * @param sock
 * @param which
 * @param arg
 */
static void cb_signal_pipe(evutil_socket_t sig, short events, void *user_data)
{
  slog(LOG_INFO, "Fired cb_signal_pipe.");

  // terminate ami handler.
  term_ami_handler();

  return;
}

/**
 * Signal handler.
 * signal: sig_hup
 * @param sock
 * @param which
 * @param arg
 */
static void cb_signal_hup(evutil_socket_t sig, short events, void *user_data)
{
  int ret;

  slog(LOG_INFO, "Fired cb_signal_hup.");

  ret = init_config();
  if(ret == false) {
    slog(LOG_DEBUG, "Could not update configration file.");
  }

  return;
}

static void cb_pid_update(int sock, short which, void* arg)
{
  struct timeval tv;
  int ret;

  slog(LOG_INFO, "Fired cb_pid_update.");

  ret = pid_update();
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update pid correctly.");
  }

  // Now reschedule timer cb
  tv.tv_sec=3;
  tv.tv_usec=0;
  event_add((struct event *) arg, &tv);
}

static bool pid_update(void)
{
  struct pidfh *pfh;
  pid_t pid;
  int ret;

  pid = getpid();

  pfh = pidfile_open(DEF_PID_FILEPATH, 0600, &pid);
  if(pfh == NULL) {
    if(errno != EEXIST) {
      slog(LOG_WARNING, "Could not open or create pidfile. pid[%jd], err[%d:%s].", (intmax_t)pid, errno, strerror(errno));
      return false;
    }
  }

  ret = pidfile_write(pfh);
  if(ret != 0) {
    slog(LOG_WARNING, "Could not update pidfile. err[%d:%s]", errno, strerror(errno));
    ret = pidfile_close(pfh);
    return false;
  }
  ret = pidfile_close(pfh);

  return true;
}

void add_event_handler(struct event* ev)
{
  int idx;

  if(ev == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }

  for(idx = 0; idx < DEF_MAX_EVENT_COUNT; idx++) {
    if(g_ev_ast[idx] != NULL) {
      continue;
    }
    g_ev_ast[idx] = ev;
    break;
  }

  return;
}
