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
#include <cjson/cJSON.h>

#include "matrix-client.h"

typedef struct MatrixBot MatrixBot;

typedef struct {
  cJSON *raw_json;

  const char *type;
  const char *sender;
  const char *event_id;
  const char *room_id;
  u_int64_t  origin_server_ts;

  cJSON *content;
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

typedef struct {
  char errcode[32];
  char error[256];
} MatrixBotLoopResult;

MatrixEvent matrixbot_event_from(cJSON *json, const char *room_id);
MatrixEventMessage matrixbot_message_from(cJSON *json);
MatrixEventMessageEdit matrixbot_messageedit_from(cJSON *json);

MatrixBot           matrixbot_new(const char *homeserver, const char *access_token);
__attribute__((warn_unused_result))
MatrixBotLoopResult matrixbot_loop(MatrixBot *bot);

#define matrixbot_qsync(bot) cJSON_Delete(matrix_sync(&bot.client, 0))

__attribute__((warn_unused_result))
MatrixSendResult matrixbot_send(MatrixBotContext ctx, const char *message);
__attribute__((warn_unused_result))
MatrixSendResult matrixbot_reply(MatrixBotContext ctx, const char *message);
__attribute__((warn_unused_result, format(printf, 2, 3)))
MatrixSendResult matrixbot_sendf(MatrixBotContext ctx, const char *fmt, ...);
__attribute__((warn_unused_result, format(printf, 2, 3)))
MatrixSendResult matrixbot_replyf(MatrixBotContext ctx, const char *fmt, ...);

#define matrixbot_qsend(ctx, message) matrixsendres_free(matrixbot_send(ctx, message))
#define matrixbot_qreply(ctx, message) matrixsendres_free(matrixbot_reply(ctx, message))
#define matrixbot_qsendf(ctx, message, ...) matrixsendres_free(matrixbot_sendf(ctx, message, __VA_ARGS__))
#define matrixbot_qreplyf(ctx, message, ...) matrixsendres_free(matrixbot_replyf(ctx, message, __VA_ARGS__))

#endif
