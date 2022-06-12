/* vim: ft=c
 *
 * matrix-bot.h
 * - part of onesunflowerbot
 *
 * id: XdAqsf9F
 */

#ifndef __XdAqsf9F_matrix_bot_h
#define __XdAqsf9F_matrix_bot_h

#include <sys/types.h>
#include <json-c/json.h>

#include "matrix-client.h"

typedef struct MatrixBot MatrixBot;

typedef struct {
  json_object *raw_json;

  const char *type;
  const char *sender;
  const char *event_id;
  const char *room_id;
  u_int64_t  origin_server_ts;

  json_object *content;
} MatrixEvent;

typedef struct {
  const char *body;
  const char *msgtype;
} MatrixEventMessage;

typedef struct {
  MatrixEventMessage message;
  const char         *changed_event_id;
} MatrixEventMessageEdit;

typedef struct {
  MatrixEvent *event;
  MatrixBot *client;
} MatrixBotContext;

typedef struct {
  void (*message_new)(
      MatrixBotContext ctx,
      MatrixEventMessage *msg);
  void (*message_edit)(
      MatrixBotContext ctx,
      MatrixEventMessageEdit *msg);
  void (*message_redact)(
      MatrixBotContext ctx,
      const char *redacted_because);
} MatrixBotHandlers;

struct MatrixBot {
  MatrixClient client;
  MatrixBotHandlers handlers;
};

MatrixEvent matrixbot_event_from(json_object *json, const char *room_id);
MatrixEventMessage matrixbot_message_from(json_object *json);
MatrixEventMessageEdit matrixbot_messageedit_from(json_object *json);

MatrixBot matrixbot_new(const char *homeserver, const char *access_token);
void      matrixbot_loop(MatrixBot *bot);

MatrixSendResult matrixbot_send(MatrixBotContext ctx, const char *message);
MatrixSendResult matrixbot_reply(MatrixBotContext ctx, const char *message);
__attribute__((format(printf, 2, 3)))
MatrixSendResult matrixbot_sendf(MatrixBotContext ctx, const char *fmt, ...);
__attribute__((format(printf, 2, 3)))
MatrixSendResult matrixbot_replyf(MatrixBotContext ctx, const char *fmt, ...);

#endif
