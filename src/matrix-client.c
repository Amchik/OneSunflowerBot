#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#include "include/matrix-client.h"

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
    char s[(sizeof(current->value) - 1) * sizeof(wchar_t) + 1];
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

/*
 * Send request to API
 */
__attribute__((nonnull(1, 2, 3))) json_object matrix_request(
    const MatrixClient *client,
    const char *method,
    const char *path,
    MatrixNode *query,
    char* body
    );

/*
 * Sync events via sending request to API
 * and modify MatrixClient
 */
__attribute__((nonnull(1))) json_object matrix_sync(
    MatrixClient *client,
    MatrixNode *query
    );
/*
 * Send event to API
 */
__attribute__((nonnull(1, 2, 3))) json_object matrix_send(
    const MatrixClient *client,
    const char *room_id,
    const char *event_type,
    char* body
    );
