
#define _GNU_SOURCE

#include <stdio.h>
#include <evhtp.h>
#include <jansson.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#include "common.h"
#include "utils.h"
#include "slog.h"
#include "resource_handler.h"
#include "http_handler.h"
#include "ami_handler.h"

//#include "ini.h"
#include "minIni.h"
#include "voicemail_handler.h"

#define DEF_SETTING_CONTEXT "general"

extern app* g_app;

static json_t* get_vm_info(const char* filename);
static json_t* get_vms_info_all(const char* context, const char* mailbox);
static json_t* get_vms_status(const char* directory, const char* status, const char* dir);
static void strip_ext(char *fname);
static char* get_vm_filename(const char* context, const char* mailbox, const char* dir, const char* msgname);

static int delete_voicemail_user(const char* context, const char* mailbox);
static bool remove_vm(const char* context, const char* mailbox, const char* dir, const char* msgname);
static int create_voicemail_user(json_t* j_data);
static int update_voicemail_user(json_t* j_data);

static char* create_voicemail_confname(void);

static bool is_exist_voicemail_user(const char* context, const char* mailbox);
static bool is_setting_context(const char* context);

static int remove_voicemail_user_info(const char* context, const char* mailbox);
static int write_voicemail_user_info(json_t* j_data);


/**
 * GET ^/voicemail/users request handler.
 * @param req
 * @param data
 */
void htp_get_voicemail_users(evhtp_request_t *req, void *data)
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

/**
 * POST ^/voicemail/users request handler.
 * @param req
 * @param data
 */
void htp_post_voicemail_users(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_post_voicemail_users.");

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    // no request data
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // create user info
  ret = create_voicemail_user(j_data);
  json_decref(j_data);
  if(ret == false) {
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/voicemail/users/?context=<context>&mailbox=<mailbox> request handler.
 * @param req
 * @param data
 */
void htp_get_voicemail_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* context;
  const char* mailbox;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_voicemail_users_detail.");

  context = evhtp_kv_find(req->uri->query, "context");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get context info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  mailbox = evhtp_kv_find(req->uri->query, "mailbox");
  if(mailbox == NULL) {
    slog(LOG_ERR, "Could not get mailbox info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_voicemail_user_info(context, mailbox);
  if(j_tmp == NULL) {
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);
  json_object_set_new(j_res, "result", j_tmp);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);
}

/**
 * PUT ^/voicemail/users/?context=<context>&mailbox=<mailbox> request handler.
 * @param req
 * @param data
 */
void htp_put_voicemail_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  json_t* j_data;
  const char* context;
  const char* mailbox;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_voicemail_users_detail.");

  context = evhtp_kv_find(req->uri->query, "context");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get context info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  mailbox = evhtp_kv_find(req->uri->query, "mailbox");
  if(mailbox == NULL) {
    slog(LOG_ERR, "Could not get mailbox info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get data
  j_data = get_json_from_request_data(req);
  if(j_data == NULL) {
    // no request data
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // update info
  ret = update_voicemail_user(j_data);
  json_decref(j_data);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not update voicemail user info.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);
}

/**
 * DELETE ^/voicemail/users/?context=<context>&mailbox=<mailbox> request handler.
 * @param req
 * @param data
 */
void htp_delete_voicemail_users_detail(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* context;
  const char* mailbox;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_delete_voicemail_users_detail.");

  context = evhtp_kv_find(req->uri->query, "context");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get context info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  mailbox = evhtp_kv_find(req->uri->query, "mailbox");
  if(mailbox == NULL) {
    slog(LOG_ERR, "Could not get mailbox info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  ret = delete_voicemail_user(context, mailbox);
  if(ret == false) {
    slog(LOG_ERR, "Could not delete voicemail user info.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // create result
  j_res = create_default_result(EVHTP_RES_OK);

  // response
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

/**
 * GET ^/voicemail/vms request handler.
 * @param req
 * @param data
 */
void htp_get_voicemail_vms(evhtp_request_t *req, void *data)
{
  json_t* j_tmp;
  json_t* j_res;
  const char* context;
  const char* mailbox;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_voicemail_mailboxes.");

  // get context
  context = evhtp_kv_find(req->uri->query, "context");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get context info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get mailbox
  mailbox = evhtp_kv_find(req->uri->query, "mailbox");
  if(mailbox == NULL) {
    slog(LOG_ERR, "Could not get mailbox info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get info
  j_tmp = get_vms_info_all(context, mailbox);
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

/**
 * GET ^/voicemail/vms/<msgname> request handler.
 * @param req
 * @param data
 */
void htp_get_voicemail_vms_msgname(evhtp_request_t *req, void *data)
{
  const char* context;
  const char* mailbox;
  const char* msgname;
  const char* dir;
  char* filename;
  int fd;
  int ret;
  struct stat sb;
  char* tmp;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_voicemail_vms_msgname.");

  // get context
  context = evhtp_kv_find(req->uri->query, "context");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get context info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get mailbox
  mailbox = evhtp_kv_find(req->uri->query, "mailbox");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get mailbox info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get dir
  dir = evhtp_kv_find(req->uri->query, "dir");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get dir info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get msgname
  msgname = req->uri->path->file;
  if(msgname == NULL) {
    slog(LOG_NOTICE, "Could not get msgname.");
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // get filename
  filename = get_vm_filename(context, mailbox, dir, msgname);
  if(filename == NULL) {
    slog(LOG_NOTICE, "Could not get filename.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }
  slog(LOG_INFO, "Get vm filename. filename[%s]", filename);

  // open file
  fd = open(filename, O_RDONLY);
  sfree(filename);
  if(fd < 0) {
    slog(LOG_NOTICE, "Could not open vm file. err[%d:%s]", errno, strerror(errno));
    if(errno == ENOENT) {
      simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    }
    else {
      simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    }
    return;
  }

  // check the file is correct
  ret = fstat(fd, &sb);
  if(ret < 0) {
    slog(LOG_ERR, "Could not get stat info. err[%d:%s]", errno, strerror(errno));
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }
  if(S_ISREG(sb.st_mode) != 1) {
//  if((sb.st_mode & S_IFREG) != 1) {
    slog(LOG_ERR, "Opened file is not voicemail file.");
    close(fd);
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // add file
  ret = evbuffer_add_file(req->buffer_out, fd, 0, sb.st_size);
  close(fd);
  if(ret != 0) {
    slog(LOG_ERR, "Could not add the file.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // reply
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Content-Type", "application/octet-stream", 0, 0));
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("Access-Control-Allow-Origin", "*", 0, 0));
  asprintf(&tmp, "attachment; filename=%s.wav", msgname);
  evhtp_headers_add_header(req->headers_out, evhtp_header_new("content-disposition", tmp, 0, 0));
  evhtp_send_reply(req, EVHTP_RES_OK);

  sfree(tmp);

  return;
}

/**
 * DELETE ^/voicemail/vms/<msgname> request handler.
 * @param req
 * @param data
 */
void htp_delete_voicemail_vms_msgname(evhtp_request_t *req, void *data)
{
  json_t* j_res;
  const char* context;
  const char* mailbox;
  const char* msgname;
  const char* dir;
  int ret;

  if(req == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return;
  }
  slog(LOG_DEBUG, "Fired htp_get_voicemail_vms_msgname.");

  // get context
  context = evhtp_kv_find(req->uri->query, "context");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get context info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get mailbox
  mailbox = evhtp_kv_find(req->uri->query, "mailbox");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get mailbox info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get dir
  dir = evhtp_kv_find(req->uri->query, "dir");
  if(context == NULL) {
    slog(LOG_ERR, "Could not get dir info.");
    simple_response_error(req, EVHTP_RES_BADREQ, 0, NULL);
    return;
  }

  // get msgname
  msgname = req->uri->path->file;
  if(msgname == NULL) {
    slog(LOG_NOTICE, "Could not get msgname.");
    simple_response_error(req, EVHTP_RES_NOTFOUND, 0, NULL);
    return;
  }

  // remove
  ret = remove_vm(context, mailbox, dir, msgname);
  if(ret == false) {
    slog(LOG_NOTICE, "Could not remove vm file.");
    simple_response_error(req, EVHTP_RES_SERVERR, 0, NULL);
    return;
  }

  // response
  j_res = create_default_result(EVHTP_RES_OK);
  simple_response_normal(req, j_res);
  json_decref(j_res);

  return;
}

static char* get_vm_filename(const char* context, const char* mailbox, const char* dir, const char* msgname)
{
  char* res;
  const char* vm_dir;

  if((context == NULL) || (mailbox == NULL) || (dir == NULL) || (msgname == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_vm_filename. context[%s], mailbox[%s], dir[%s], msgname[%s]",
      context,
      mailbox,
      dir,
      msgname
      );

  vm_dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "voicemail"), "dicretory"));
  if(vm_dir == NULL) {
    slog(LOG_ERR, "Could not get voicemail directory setting.");
    return NULL;
  }

  asprintf(&res, "%s/%s/%s/%s/%s.wav", vm_dir, context, mailbox, dir, msgname);

  return res;
}

static json_t* get_vm_info(const char* filename)
{
  json_t* j_res;
  json_t* j_tmp;
  int idx;
  char tmp_data[2048];
  const char* tmp_const;
  json_t* j_message_items;

  if(filename == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  /**
   * voicemail [message] section items.
   */
  j_message_items = json_pack(
      "["
        "s, s, s, s, s, "
        "s, s, s, s, s, "
        "s, s, s, s"
      "]",

      "origmailbox",
      "context",
      "macrocontext",
      "exten",
      "rdnis",

      "priority",
      "callerchan",
      "callerid",
      "origdate",
      "origtime",

      "category",
      "msg_id",
      "flag",
      "duration"
      );

  // get [message] info.
  j_res = json_object();
  json_array_foreach(j_message_items, idx, j_tmp) {
    tmp_const = json_string_value(j_tmp);
    ini_gets("message", tmp_const, "", tmp_data, sizeof(tmp_data), filename);
    json_object_set_new(j_res, tmp_const, json_string(tmp_data));
  }

  return j_res;
}

/**
 @brief strip extension
 */
static void strip_ext(char *fname)
{
  char *end = fname + strlen(fname);

  while ((end > fname) && (*end != '.') && (*end != '\\') && (*end != '/')) {
    --end;
  }

  if ((end > fname) && (*end == '.')) {
    *end = '\0';
  }
}

/**
 * Get given mailbox@context's all vm info
 * @param context
 * @param mailbox
 * @return
 */
static json_t* get_vms_info_all(const char* context, const char* mailbox)
{
  json_t* j_res;
  json_t* j_res_tmp;
  json_t* j_targets;
  json_t* j_target;
  char* directory;
  const char* status;
  const char* dir;
  const char* vm_dir;
  int idx;

  if((context == NULL) || (mailbox == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  vm_dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "voicemail"), "dicretory"));
  if(vm_dir == NULL) {
    slog(LOG_ERR, "Could not get voicemail directory info.");
    return NULL;
  }

  // create targets
  j_targets = json_pack("["
      "{s:s, s:s}, "
      "{s:s, s:s}"
    "]",

    "dir", "INBOX",
    "status", "unread",

    "dir", "Old",
    "status", "read"
  );

  j_res = json_array();
  json_array_foreach(j_targets, idx, j_target) {
    // get target info
    dir = json_string_value(json_object_get(j_target, "dir"));
    status = json_string_value(json_object_get(j_target, "status"));

    // create directory path
    asprintf(&directory, "%s/%s/%s/%s",
        vm_dir,
        context,
        mailbox,
        dir
    );
    slog(LOG_DEBUG, "Created target directory. directory[%s]", directory);

    // get vms info
    j_res_tmp = get_vms_status(directory, status, dir);
    sfree(directory);

    // append result
    json_array_extend(j_res, j_res_tmp);
    json_decref(j_res_tmp);
  }
  json_decref(j_targets);

  return j_res;
}

/**
 @brief Remove voicemail files of given info.
*/
static bool remove_vm(const char* context, const char* mailbox, const char* dir, const char* msgname)
{
  json_t* j_tmp;
  char* filename;
  const char* vm_dir;
  const char* exts[] = {"gsm", "txt", "wav", "WAV"};
  int ret;
  int i;

  if((context == NULL) || (mailbox == NULL) || (dir == NULL) || (msgname == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }
  slog(LOG_WARNING, "Fired remove_vm. context[%s], mailbox[%s], dir[%s], msgname[%s]", context, mailbox, dir, msgname);

  vm_dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "voicemail"), "dicretory"));
  if(vm_dir == NULL) {
    slog(LOG_ERR, "Could not get voicemail directory info.");
    return false;
  }

  for(i = 0; i < (sizeof(exts)/sizeof(exts[0])); i++) {
    // create target filename
    asprintf(&filename, "%s/%s/%s/%s/%s.%s", vm_dir, context, mailbox, dir, msgname, exts[i]);
    slog(LOG_DEBUG, "Remove file. filename[%s]", filename);
    remove(filename);
    free(filename);
  }

  // create refresh request
  j_tmp = json_pack("{s:s, s:s, s:s}",
      "Action",   "VoicemailRefresh",
      "Context",  context,
      "Mailbox",  mailbox
      );

  // send refresh request
  ret = send_ami_cmd(j_tmp);
  json_decref(j_tmp);
  if(ret == false) {
    slog(LOG_ERR, "Could not send ami action for voicemail refresh. action[%s]", "Hangup");
    // we alreay delete file. So, we don't return the false at this point.
    // just write log only.
    // return false;
  }

  return true;
}

/**
 * Get all vm's status in given directory
 * @param directory
 * @param status  Set status value
 * @param dir     Set dir value
 * @return
 */
static json_t* get_vms_status(const char* directory, const char* status, const char* dir)
{
  json_t* j_res;
  json_t* j_tmp;
  char* filename;
  char* msgname;
  DIR* d_dir;
  struct dirent* ent;
  char* tmp;

  if((directory == NULL) || (status == NULL) || (dir == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }
  slog(LOG_DEBUG, "Fired get_vms_status. directory[%s], status[%s]", directory, status);

  d_dir = opendir(directory);
  if(d_dir == NULL) {
    slog(LOG_ERR, "Could not open directory. err[%d:%s]", errno, strerror(errno));
    return NULL;
  }

  j_res = json_array();
  while(1) {
    ent = readdir(d_dir);
    if(ent == NULL) {
      break;
    }

    // is target file
    tmp = strstr(ent->d_name, ".txt");
    if(tmp == NULL) {
      continue;
    }

    // create filename
    asprintf(&filename, "%s/%s", directory, ent->d_name);

    // get vm info
    j_tmp = get_vm_info(filename);
    sfree(filename);
    if(j_tmp == NULL) {
      slog(LOG_ERR, "Could not parse the voicemail info. err[%d:%s]", errno, strerror(errno));
      continue;
    }

    // set status
    json_object_set_new(j_tmp, "status", json_string(status));

    // set dir
    json_object_set_new(j_tmp, "dir", json_string(dir));

    // set msgname
    msgname = strdup(ent->d_name);
    strip_ext(msgname);
    json_object_set_new(j_tmp, "msgname", json_string(msgname));
    sfree(msgname);

    // append
    json_array_append_new(j_res, j_tmp);
  }
  closedir(d_dir);

  return j_res;
}

static char* create_voicemail_user_info_string(json_t* j_data)
{
  // check mandatory items
  char* tmp_str;
  char* res;
  json_t* j_options;
  json_t* j_option;
  json_t* j_value;
  const char* key;
  const char* option;
  int type;
  int idx;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return NULL;
  }

  // mandatory items
  asprintf(&res, "%s,%s,%s,%s,",
    json_string_value(json_object_get(j_data, "password"))? : "",
    json_string_value(json_object_get(j_data, "full_name"))? : "",
    json_string_value(json_object_get(j_data, "email"))? : "",
    json_string_value(json_object_get(j_data, "pager"))? : ""
    );

  j_options = json_pack("["
        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "
        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "
        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "
        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "
        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "

        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "
        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "
        "{s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, {s:s, s:s}, "

        "{s:s, s:s} "

      "]",


      "key", "timezone", "option", "tz",
      "key", "locale", "option", "locale",
      "key", "attach_message", "option", "attach",
      "key", "attachment_format", "option", "attachfmt",
      "key", "say_cid", "option", "saycid",

      "key", "cid_internal_contexts", "option", "cidinternalcontexts",
      "key", "say_duration", "option", "sayduration",
      "key", "say_duration_minimum", "option", "saydurationm",
      "key", "dialout", "option", "dialout",
      "key", "send_voicemail", "option", "send_voicemail",

      "key", "search_contexts", "option", "searchcontexts",
      "key", "callback", "option", "callback",
      "key", "exit_context", "option", "exitcontext",
      "key", "can_review", "option", "review",
      "key", "call_operator", "option", "operator",

      "key", "say_envelope", "option", "envelope",
      "key", "delete_message", "option", "delete",
      "key", "alias", "option", "alias",
      "key", "volume_gain", "option", "volgain",
      "key", "next_after_cmd", "option", "nextaftercmd",

      "key", "force_name", "option", "forcename",
      "key", "force_greetings", "option", "forcegreetings",
      "key", "hide_from_dir", "option", "hidefromdir",
      "key", "temp_greet_warn", "option", "tempgreetwarn",
      "key", "password_location", "option", "passwordlocation",

      "key", "message_wrap", "option", "messagewrap",
      "key", "min_password", "option", "minpassword",
      "key", "vm_password", "option", "vm-password",
      "key", "vm_new_password", "option", "vm-newpassword",
      "key", "vm_pass_changed", "option", "vm-passchanged",

      "key", "vm_reenter_password", "option", "vm-reenterpassword",
      "key", "vm_mismatch", "option", "vm-mismatch",
      "key", "vm_invalid_password", "option", "vm-invalid-password",
      "key", "vm_pls_try_again", "option", "vm-pls-try-again",
      "key", "vm_prepend_timeout", "option", "vm-prepend-timeout",

      "key", "listen_control_forward_key", "option", "listen-control-forward-key",
      "key", "listen_control_reverse_key", "option", "listen-control-reverse-key",
      "key", "listen_control_pause_key", "option", "listen-control-pause-key",
      "key", "listen_control_restart_key", "option", "listen-control-restart-key",
      "key", "listen_control_stop_key", "option", "listen-control-stop-key",

      "key", "backup_deleted", "option", "backupdeleted" // last item. backupdeleted
      );

  json_array_foreach(j_options, idx, j_option) {
    // get key
    key = json_string_value(json_object_get(j_option, "key"));

    // get value from data
    j_value = json_object_get(j_data, key);
    if(j_value == NULL) {
      continue;
    }

    // get option
    option = json_string_value(json_object_get(j_option, "option"));

    tmp_str = NULL;
    type = json_typeof(j_value);
    if(type == JSON_STRING) {
      asprintf(&tmp_str, "%s%s=%s|", res, option, json_string_value(j_value));
    }
    else if(type == JSON_INTEGER) {
      asprintf(&tmp_str, "%s%s=%"JSON_INTEGER_FORMAT"|", res, option, json_integer_value(j_value));
    }
    else if(type == JSON_REAL) {
      asprintf(&tmp_str, "%s%s=%f|", res, option, json_real_value(j_value));
    }
    else if(type == JSON_TRUE) {
      asprintf(&tmp_str, "%s%s=%s|", res, option, "true");
    }
    else if(type == JSON_FALSE) {
      asprintf(&tmp_str, "%s%s=%s|", res, option, "false");
    }
    else {
      // wrong parameter.
      continue;
    }
    sfree(res);
    res = tmp_str;
  }
  json_decref(j_options);

  return res;
}

static bool is_exist_voicemail_user(const char* context, const char* mailbox)
{
  json_t* j_tmp;

  if((context == NULL) || (mailbox == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  j_tmp = get_voicemail_user_info(context, mailbox);
  if(j_tmp == NULL) {
    return false;
  }
  json_decref(j_tmp);

  return true;
}

static bool is_setting_context(const char* context)
{
  int ret;

  if(context == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  ret = strcmp(context, DEF_SETTING_CONTEXT);
  if(ret == 0) {
    return true;
  }

  return false;
}

/**
 * Write given voicemail user info to the voicemail configuration file.
 * @param j_data
 * @return
 */
static int write_voicemail_user_info(json_t* j_data)
{
  char* user_info;
  char* confname;
  const char* context;
  const char* mailbox;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // get madatory item
  context = json_string_value(json_object_get(j_data, "context"));
  mailbox = json_string_value(json_object_get(j_data, "mailbox"));
  if((mailbox == NULL) || (context == NULL)) {
    slog(LOG_ERR, "Could not get mailbox info.");
    return false;
  }

  // is setting context?
  ret = is_setting_context(context);
  if(ret == true) {
    slog(LOG_NOTICE, "Given context is setting context.");
    return false;
  }

  // create user string
  user_info = create_voicemail_user_info_string(j_data);
  if(user_info == NULL) {
    slog(LOG_ERR, "Could not create user string.");
    return false;
  }

  confname = create_voicemail_confname();
  if(confname == NULL) {
    slog(LOG_ERR, "Could not create voicemail configuration filename.");
    return false;
  }

  ini_puts(context, mailbox, user_info, confname);
  sfree(user_info);
  sfree(confname);

  return true;
}

/**
 * Revmoe given voicemail user info from the voicemail configuration file.
 * @param j_data
 * @return
 */
static int remove_voicemail_user_info(const char* context, const char* mailbox)
{
  char* confname;
  int ret;

  if((context == NULL) || (mailbox == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // is setting context?
  ret = is_setting_context(context);
  if(ret == true) {
    slog(LOG_NOTICE, "Given context is setting context.");
    return false;
  }

  confname = create_voicemail_confname();
  if(confname == NULL) {
    slog(LOG_ERR, "Could not create voicemail configuration filename.");
    return false;
  }

  // remove it!
  ini_puts(context, mailbox, NULL, confname);
  sfree(confname);

  return true;
}


/**
 * Update voicemail user info.
 *
 * @param j_data
 * @return
 */
static int update_voicemail_user(json_t* j_data)
{
  int ret;
  const char* context;
  const char* mailbox;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  context = json_string_value(json_object_get(j_data, "context"));
  mailbox = json_string_value(json_object_get(j_data, "mailbox"));
  if((context == NULL) || (mailbox == NULL)) {
    slog(LOG_ERR, "Could not get context or malbox info.");
    return false;
  }

  // check existence
  ret = is_exist_voicemail_user(context, mailbox);
  if(ret != true) {
    slog(LOG_NOTICE, "The given voicemail user info is not exist. context[%s], malbox[%s]", context, mailbox);
    return false;
  }

  // update
  ret = write_voicemail_user_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update voicemail user info.");
    return false;
  }

  return true;
}

/**
 * Create voicemail_user info.
 * @return
 */
static int create_voicemail_user(json_t* j_data)
{
  const char* context;
  const char* mailbox;
  int ret;

  if(j_data == NULL) {
    slog(LOG_WARNING, "Wrong input parameter\n");
    return false;
  }
  slog(LOG_DEBUG, "Fired create_voicemail_user");

  // get madatory item
  context = json_string_value(json_object_get(j_data, "context"));
  mailbox = json_string_value(json_object_get(j_data, "mailbox"));
  if((mailbox == NULL) || (context == NULL)) {
    slog(LOG_ERR, "Could not get mailbox info.");
    return false;
  }

  // check existence
  ret = is_exist_voicemail_user(context, mailbox);
  if(ret != false) {
    slog(LOG_NOTICE, "The given voicemail user info is already exist. context[%s], malbox[%s]", context, mailbox);
    return false;
  }

  // create
  ret = write_voicemail_user_info(j_data);
  if(ret == false) {
    slog(LOG_ERR, "Could not update voicemail user info.");
    return false;
  }

  return true;
}

/**
 * Delete voicemail user info.
 *
 * @param j_data
 * @return
 */
static int delete_voicemail_user(const char* context, const char* mailbox)
{
  int ret;

  if((context == NULL) || (mailbox == NULL)) {
    slog(LOG_WARNING, "Wrong input parameter.");
    return false;
  }

  // check existence
  ret = is_exist_voicemail_user(context, mailbox);
  if(ret != true) {
    slog(LOG_NOTICE, "The given voicemail user info is not exist. context[%s], malbox[%s]", context, mailbox);
    return false;
  }

  // remove it!
  ret = remove_voicemail_user_info(context, mailbox);
  if(ret == false) {
    slog(LOG_ERR, "Could not remove voicemail user info.");
    return false;
  }

  return true;
}

static char* create_voicemail_confname(void)
{
  char* res;
  const char* dir;
  const char* conf;

  dir = json_string_value(json_object_get(json_object_get(g_app->j_conf, "general"), "directory_conf"));
  conf = json_string_value(json_object_get(json_object_get(g_app->j_conf, "voicemail"), "conf_name"));
  if((dir == NULL) || (conf == NULL)) {
    slog(LOG_ERR, "Could not get voicemail config file info.");
    return NULL;
  }

  asprintf(&res, "%s/%s", dir, conf);
  return res;
}
