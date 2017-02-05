/*
 * event_handler.c
 *
 *  Created on: Feb 5, 2017
 *      Author: pchero
 */


#include <event2/event.h>
#include <signal.h>
#include <stdbool.h>

#include "common.h"
#include "slog.h"
#include "event_handler.h"

extern app* g_app;
extern struct event_base*  g_base;

static void cb_signal_term(int sock, short which, void* arg);

static void cb_signal_term(int sock, short which, void* arg)
{
  slog(LOG_NOTICE, "Fired cb_signal_term.");
  event_base_loopbreak(g_base);

  return;
}

bool init_event_handler(void)
{
  struct event* ev;

  // add SIGTERM event handler
  ev = event_new(g_base, SIGTERM, EV_SIGNAL | EV_PERSIST, cb_signal_term, NULL);
  event_add(ev, NULL);

  return true;
}
