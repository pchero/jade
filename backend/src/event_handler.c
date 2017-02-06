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

#define DEF_PID_FILEPATH "/var/run/jade_backend.pid"
#define DEF_PID_TIMEOUT_SEC 5

extern app* g_app;
extern struct event_base*  g_base;

static void cb_signal_term(int sock, short which, void* arg);
static void cb_signal_int(int sock, short which, void* arg);
static void cb_pid_update(int sock, short which, void* arg);

static bool pid_update(void);

bool init_event_handler(void)
{
  struct event* ev;
  struct event* ev_pid;
  struct timeval tv;

  // add signal handler
  ev = event_new(g_base, SIGTERM, EV_SIGNAL | EV_PERSIST, cb_signal_term, NULL);
  event_add(ev, NULL);

  ev = event_new(g_base, SIGINT, EV_SIGNAL | EV_PERSIST, cb_signal_int, NULL);
  event_add(ev, NULL);

  // add pid update
  tv.tv_sec = DEF_PID_TIMEOUT_SEC;
  tv.tv_usec = 0;
  ev_pid = calloc(sizeof(struct event), 1);
  event_assign(ev_pid, g_base, -1, 0, cb_pid_update, ev_pid);
  event_add(ev_pid, &tv);

  return true;
}

static void cb_signal_term(int sock, short which, void* arg)
{
  slog(LOG_INFO, "Fired cb_signal_term.");
  event_base_loopbreak(g_base);

  return;
}

static void cb_signal_int(int sock, short which, void* arg)
{
  slog(LOG_INFO, "Fired cb_signal_int.");
  event_base_loopbreak(g_base);

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
