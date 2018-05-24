

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <jansson.h>
#include <unistd.h>

#include "common.h"
#include "slog.h"
#include "config.h"
#include "db_ctx_handler.h"
#include "http_handler.h"
#include "event_handler.h"
#include "data_handler.h"
#include "resource_handler.h"
#include "misc_handler.h"
#include "ob_event_handler.h"
#include "zmq_handler.h"
#include "websocket_handler.h"
#include "conf_handler.h"
#include "user_handler.h"
#include "chat_handler.h"
#include "call_handler.h"
#include "core_handler.h"
#include "dialplan_handler.h"

#include "park_handler.h"
#include "queue_handler.h"
#include "pjsip_handler.h"
#include "sip_handler.h"

#include "me_handler.h"
#include "admin_handler.h"
#include "manager_handler.h"

app* g_app;


static bool option_parse(int argc, char** argv);
static void print_help(void);

/**
 * Initiate jade_backend
 * @return
 */
bool init(void)
{
  int ret;
  
  g_app = calloc(sizeof(app), 1);
  g_app->j_conf = NULL;
  g_app->evt_base = NULL;

  ret = slog_init_handler();
  if(ret == false) {
    printf("Failed initiate log.");
    return false;
  }
  slog(LOG_NOTICE, "Finished init_log.");

  ret = config_init();
  if(ret == false) {
    printf("Could not initiate config.");
    return false;
  }
  slog(LOG_NOTICE, "Finished init_config.");

  ret = event_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate event_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_event_handler.");

  ret = resource_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate resource.");
    return false;
  }
  slog(LOG_NOTICE, "Finished resource_init.");

  ret = zmq_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Coudl not initiate zmq_handler.");
    return false;
  }

  ret = data_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate ami_handle.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_ami_handler.");

  ret = http_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate http_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_http_handler.");
  
  ret = websocket_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate websocket_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_websocket_handler.");

//  ret = ob_init_handler();
//  if(ret == false) {
//    slog(LOG_ERR, "Could not initiate ob_handler.");
//    return false;
//  }
//  slog(LOG_DEBUG, "Finished init_outbound.");

  ret = conf_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate conf_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_conf_handler.");

  ret = core_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate call_handler.");
    return false;
  }

  ret = user_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate user_handler");
    return false;
  }

  ret = misc_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate misc_handler.");
    return false;
  }

  // other module handlers
  ret = park_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate park_handler.");
    return false;
  }

  ret = queue_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate queue_handler.");
    return false;
  }

  ret = pjsip_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initate pjsip_handler.");
    return false;
  }

  ret = sip_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate sip_handler.");
    return false;
  }

  ret = chat_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate chat_handler.");
    return false;
  }

  ret = dialplan_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate dialplan_handler.");
    return false;
  }

  ret = me_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate me_handler.");
    return false;
  }

  ret = admin_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate admin_handler.");
    return false;
  }

  ret = manager_init_handler();
  if(ret == false) {
    slog(LOG_ERR, "Could not initiate manager_handler.");
    return false;
  }

  return true;
}

bool terminate(void)
{
  slog(LOG_INFO, "Terminating..");

  http_term_handler();

  // terminate event handler
  event_term_handler();

  // terminate outbound module.
  ob_term_handler();

  // terminate zmq
  zmq_term_handler();

  websocket_term_handler();

  resource_term_handler();

  // terminate modules
  me_term_handler();
  admin_term_handler();
  manager_term_handler();

  event_base_free(g_app->evt_base);
  g_app->evt_base = NULL;

  return true;
}

int main(int argc, char** argv) 
{
  int ret;
  
  // parse options
  ret = option_parse(argc, argv);
  if(ret == false) {
    return 1;
  }

  // initiate
  ret = init();
  if(ret == false) {
    printf("Could not initiate.\n");
    return 1;
  }
  slog(LOG_INFO, "Started backend service.");
    
  event_base_loop(g_app->evt_base, 0);

  terminate();

  return 0;
}

static void print_help(void)
{
  printf("Usage: jade_backend [OPTIONS]\n");
  printf("Valid options:\n");
  printf("  -h              : This help screen.\n");
  printf("  -f <configfile> : Use an alternate configuration file.\n");
  printf("\n");
  return;
}

/**
 * Parse option.
 * @param argc
 * @param argv
 * @return
 */
static bool option_parse(int argc, char** argv)
{
  char opt;

  if(argc > 1) {
    opt = getopt(argc, argv, "hc:");
    if(opt == -1) {
      print_help();
      return false;
    }

    switch(opt) {
      case 'f':
      {
        config_update_filename(optarg);
      }
      break;

      case 'h':
      default:
      {
        print_help();
        return false;
      }
    }
  }
  return true;
}
