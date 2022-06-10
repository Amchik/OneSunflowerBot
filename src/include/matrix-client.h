/* vim: ft=c
 *
 * matrix-client.h
 * - part of onesunflowerbot
 *
 * id: uoQ7JFxV
 */

#ifndef __uoQ7JFxV_matrix_client_h
#define __uoQ7JFxV_matrix_client_h

#include <json-c/json.h>
#include <wchar.h>

#include "matrix-defines.h"

typedef struct MatrixClient {
  char batch[64];
  char homeserver[64]; /* yes, domains like ohmygodthisdomainistoolong.com not supported */
  char access_token[MATRIX_ACCESSTOKEN_LEN];
} MatrixClient;

typedef struct MatrixNode {
  char *key;
  wchar_t *value;
  unsigned char flags;
  struct MatrixNode *next;
} MatrixNode;

#define matrix_newnode(_key, _value, _flags, _next) \
  ((MatrixNode){ .key = _key, .value = _value, .flags = _flags, .next = _next })

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

#endif

