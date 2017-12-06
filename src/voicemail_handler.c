
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

#include "ini.h"
#include "voicemail_handler.h"

extern app* g_app;


static json_t* get_vms_status(const char* directory, const char* status, const char* dir);
static json_t* get_vms_info_all(const char* context, const char* mailbox);
static int vm_info_parser(void* data, const char* section, const char* name, const char* value);
static void strip_ext(char *fname);
static char* get_vm_filename(const char* context, const char* mailbox, const char* dir, const char* msgname);
static bool remove_vm(const char* context, const char* mailbox, const char* dir, const char* msgname);

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
  if(context == NULL) {
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

/**
 * Voicemail status file parser
 * @param data
 * @param section
 * @param name
 * @param value
 * @return
 */
static int vm_info_parser(
    void* data,
    const char* section,
    const char* name,
    const char* value
    )
{
  json_t* j_data;

  j_data = (json_t*)data;

  json_object_set_new(j_data, name, json_string(value));
  return 1;
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
  slog(LOG_DEBUG, "Check value. size_1[%lu], size_2[%lu]", sizeof(exts), sizeof(exts[0]));

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
  int ret;
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

    // parse
    j_tmp = json_object();
    ret = ini_parse(filename, vm_info_parser, j_tmp);
    sfree(filename);
    if(ret != 0) {
      slog(LOG_ERR, "Could not parse the file correctly. err[%d:%s]", errno, strerror(errno));
      json_decref(j_tmp);
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

