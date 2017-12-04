
#include <stdio.h>
#include <evhtp.h>
#include <jansson.h>

#include "slog.h"
#include "resource_handler.h"
#include "http_handler.h"

#include "voicemail_handler.h"

/**
 * GET ^/voicemail/mailboxes request handler.
 * @param req
 * @param data
 */
void htp_get_voicemail_mailboxes(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_voicemail_mailboxes.");

  // get info
  j_tmp = get_voicemail_users_all();
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", json_object());
  json_object_set_new(json_object_get(j_res, "result"), "list", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}
