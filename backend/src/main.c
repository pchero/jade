

#include <stdio.h>
#include <stdbool.h>
#include <event2/event.h>
#include <evhtp.h>
#include <jansson.h>
#include <unistd.h>

#include "common.h"
#include "slog.h"
#include "config.h"
#include "db_handler.h"
#include "http_handler.h"
#include "event_handler.h"
#include "data_handler.h"

#include "ob_event_handler.h"


app* g_app;
struct event_base*  g_base = NULL;

static bool init_event(void);
static bool option_parse(int argc, char** argv);
static void print_help(void);


static bool init_event(void)
{
  if(g_base == NULL) {
    printf("New event base.\n");
    g_base = event_base_new();
  }

  return true;
}

/**
 * Initiate jade_backend
 * @return
 */
bool init(void)
{
  int ret;
  
  g_app = calloc(sizeof(app), 1);
  g_app->j_conf = NULL;

  ret = init_log();
  if(ret == false) {
    printf("Failed initiate log.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_log.");

  ret = init_config();
  if(ret == false) {
    printf("Could not initiate config.");
    return false;
  }

  ret = init_event();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate event.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_event.");

  ret = db_init();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate db.");
    return false;
  }
  slog(LOG_DEBUG, "Finished db_init.");

  ret = init_data_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate ami_handle.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_ami_handler.");

  ret = init_http_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate http_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_http_handler.");
  
  ret = init_event_handler();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate event_handler.");
    return false;
  }
  slog(LOG_DEBUG, "Finished init_event_handler.");

  ret = init_outbound();
  if(ret == false) {
    slog(LOG_WARNING, "Could not initiate ob_handler.");
    return false;
  }

  return true;
}

bool terminate(void)
{
  slog(LOG_INFO, "Terimnating..");

  term_http_handler();
  db_term();

  event_base_free(g_base);

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
    
  event_base_loop(g_base, 0);

  terminate();

  return 0;
}

static void print_help(void)
{
  printf("Usage: jade_backend [OPTIONS]\n");
  printf("Valid options:\n");
  printf("  -h              : This help screen.\n");
  printf("  -c <configfile> : Use an alternate configuration file.\n");
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
        update_config_filename(optarg);
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
