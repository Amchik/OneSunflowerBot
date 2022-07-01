/* vim: ft=c
 *
 * matrix-client.h
 * - part of onesunflowerbot
 *
 * id: uoQ7JFxV
 */

#ifndef __uoQ7JFxV_matrix_client_h
#define __uoQ7JFxV_matrix_client_h

#include <cjson/cJSON.h>

#include "matrix-defines.h"

typedef struct MatrixClient {
  char batch[64];
  char homeserver[64]; /* yes, domains like ohmygodthisdomainistoolong.com not supported */
  char access_token[MATRIX_ACCESSTOKEN_LEN];
} MatrixClient;

typedef struct MatrixNode {
  const char *key;
  const char *value;
  unsigned char flags;
  const struct MatrixNode *next;
} MatrixNode;

typedef struct {
  cJSON *raw_json;

  const char *errcode;
  const char *error;

  const char *event_id;
} MatrixSendResult;

#define matrix_newnode(_key, _value, _flags, _next) \
  ((MatrixNode){ .key = _key, .value = _value, .flags = _flags, .next = _next })
#define matrixsendres_free(res) \
  cJSON_Delete(res.raw_json)

/*
 * Stringify matrix nodes into json object.
 * Allocates string.
 */
__attribute__((malloc))
char* matrixnode_stringify(MatrixNode chain);

/*
 * Free matrix nodes if needed (see $.flags)
 */
void matrixnode_free(MatrixNode chain);

/*
 * Send request to API
 */
__attribute__((nonnull(1, 2, 3))) cJSON* matrix_request(
    const MatrixClient *client,
    const char *method,
    const char *path,
    const MatrixNode *query,
    const char* body
    );

/*
 * Sync events via sending request to API
 * and modify MatrixClient
 */
__attribute__((nonnull(1))) cJSON* matrix_sync(
    MatrixClient *client,
    const MatrixNode *query
    );
/*
 * Send room state to API
 */
__attribute__((nonnull(1, 2, 3))) MatrixSendResult matrix_state(
    const MatrixClient *client,
    const char *room_id,
    const char *event_type,
    const char* body
    );
/*
 * Send event to API
 */
__attribute__((nonnull(1, 2, 3))) MatrixSendResult matrix_send(
    const MatrixClient *client,
    const char *room_id,
    const char *event_type,
    const char* body
    );
/*
 * Redact event in API
 */
__attribute__((nonnull(1, 2, 3))) MatrixSendResult matrix_redact(
    const MatrixClient *client,
    const char *room_id,
    const char *event_id,
    const char* reason
    );

#endif

