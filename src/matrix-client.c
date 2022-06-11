#include <curl/urlapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <curl/curl.h>
#include <curl/easy.h>

#include "include/matrix-client.h"

struct MatrixCurlString {
  char *str;
  size_t size, used;
};

__attribute__((malloc))
char* matrixnode_stringify(MatrixNode chain) {
  json_object *json;
  MatrixNode *current;
  const char *answer;
  char *realanswer;
  size_t answer_len;

  json = json_object_new_object();
  current = &chain;
  while (current != 0) {
    char s[(wcslen(current->value) - 1) * sizeof(wchar_t) + 1];
    s[0] = 0;
    sprintf(s, "%ls", current->value);

    json_object_object_add(json, current->key, json_object_new_string(s));
    current = current->next;
  }

  answer = json_object_to_json_string(json);
  answer_len = strlen(answer) + 1;
  realanswer = malloc(answer_len);
  memcpy(realanswer, answer, answer_len);

  /* this function also free answer */
  json_object_put(json);

  return(realanswer);
}

void matrixnode_free(MatrixNode chain) {
#define is(f) ((current->flags & f) == f)
  MatrixNode *current, *next;

  current = &chain;
  while (current != 0) {
    if is(MATRIXNODE_FREEKEY) {
      free(current->key);
    }
    if is(MATRIXNODE_FREEVALUE) {
      free(current->value);
    }

    if is(MATRIXNODE_FREENODE) {
      next = current->next;
      free(current);
      current = next;
    } else {
      current = current->next;
    }
  }
#undef is
}

size_t matrixcurl_readcallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  char **str;
  size_t slen, plen;
  curl_off_t nread;

  str = (char**)userdata;
  plen = size * nmemb;
  slen = strlen(*str); /* need to optimize it... */
  nread = plen > slen ? slen : plen;

  memcpy(ptr, *str, nread);
  *str = *str + nread;

  return(nread);
}
size_t matrixcurl_writecallback(void *data, size_t size, size_t nmemb, void *userp) {
  struct MatrixCurlString *str;
  size_t realsize;

  str = (struct MatrixCurlString*)userp;
  realsize = size * nmemb;
  if (realsize > (str->size - str->used)) {
    str->size = realsize + str->used;
    str->str = realloc(str->str, str->size + 1);
  }
  memcpy(str->str + str->used, data, realsize);
  str->used += realsize;

  return(realsize);
}

__attribute__((nonnull(1, 2, 3))) json_object* matrix_request(
    const MatrixClient *client,
    const char *method,
    const char *path,
    MatrixNode *query,
    char* body
    ) {
  CURL *curl;
  CURLU *url;
  struct curl_slist *headers;
  char *realpath, authorization[sizeof(client->access_token) + 22], *mutbody;
  MatrixNode *qcurrent;
  struct MatrixCurlString str;
  json_object *json;

  snprintf(authorization, sizeof(authorization), "Authorization: Bearer %s",
      client->access_token);

  curl = curl_easy_init();
  url = curl_url();
  headers = 0;
  mutbody = body;
  str.size = 1024;
  str.used = 0;
  str.str = malloc(str.size + 1);

  curl_url_set(url, CURLUPART_SCHEME, "https", 0);
  curl_url_set(url, CURLUPART_HOST, client->homeserver, 0);
  curl_url_set(url, CURLUPART_PATH, path, 0);
  
  for (qcurrent = query; qcurrent != 0; qcurrent = qcurrent->next) {
    char buff[512];

    buff[0] = 0;
    snprintf(buff, sizeof(buff), "%s=%ls", qcurrent->key, qcurrent->value);

    curl_url_set(url, CURLUPART_QUERY, buff, CURLU_URLENCODE | CURLU_APPENDQUERY);
  }

  curl_url_get(url, CURLUPART_URL, &realpath, 0);
  curl_url_cleanup(url);

  headers = curl_slist_append(headers, authorization);
  headers = curl_slist_append(headers, "Content-Type: application/json"); /* if needed? */

  curl_easy_setopt(curl, CURLOPT_URL, realpath);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, matrixcurl_writecallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
  if (body != 0 && body[0] != '\0') {
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, matrixcurl_readcallback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &mutbody);
  }

  curl_easy_perform(curl);

  str.str[str.used + 1] = 0;
  if (str.size > 20000) {
    printf("[POSSIBLE IOT TRAP] str.size = %lu (safe value - 0~20000)\n", str.size);
  }
  json = json_tokener_parse(str.str);

  free(str.str);
  curl_easy_cleanup(curl);
  curl_free(realpath);
  curl_slist_free_all(headers);

  return(json);
}

__attribute__((nonnull(1))) json_object* matrix_sync(
    MatrixClient *client,
    MatrixNode *query
    ) {
  json_object *json;
  wchar_t buff[64];
  const char *next_batch;
  MatrixNode *fullquery;

  if (client->batch[0] != '\0') {
    swprintf(buff, sizeof(buff), L"%s", client->batch);
    fullquery = &matrix_newnode("since", buff, 0, query);
  } else {
    fullquery = query;
  }

  json = matrix_request(client, "GET", "/_matrix/client/v3/sync", fullquery, 0);
  if (json == 0) return(json);

  next_batch = json_object_get_string(json_object_object_get(json, "next_batch"));
  strncpy(client->batch, next_batch, 64);

  return(json);
}

__attribute__((nonnull(1, 2, 3))) json_object* matrix_send(
    const MatrixClient *client,
    const char *room_id,
    const char *event_type,
    char* body
    ) {
  json_object *json;
  char path[256];

  snprintf(path, sizeof(path), "/_matrix/client/v3/rooms/%s/send/%s/%d%d", room_id, event_type, rand(), rand());

  json = matrix_request(client, "PUT", path, 0, body);

  return(json);
}
