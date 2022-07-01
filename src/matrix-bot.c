#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <cjson/cJSON.h>

#include "include/matrix-defines.h"
#include "include/matrix-client.h"
#include "include/matrix-bot.h"

#define MATRIXBOT_SETPROPU(res, orjson, json, prop)\
  json = cJSON_GetObjectItemCaseSensitive(orjson, #prop);\
  if (json)\
    res.prop = json->valuestring;\
  else\
    res.prop = 0
#define MATRIXBOT_SETPROP(res, json, prop) MATRIXBOT_SETPROPU(res, res.raw_json, json, prop)

MatrixEvent matrixbot_event_from(cJSON *json, const char *room_id) {
  MatrixEvent res;

  res.raw_json = json;
  MATRIXBOT_SETPROP(res, json, type);
  MATRIXBOT_SETPROP(res, json, sender);
  MATRIXBOT_SETPROP(res, json, event_id);
  if (room_id) {
    res.room_id = room_id;
  } else {
    MATRIXBOT_SETPROP(res, json, room_id);
  }
  json = cJSON_GetObjectItemCaseSensitive(res.raw_json, "origin_server_ts");
  if (json)
    res.origin_server_ts = (u_int64_t)json->valuedouble; /* note: this cringe uses because matrix returns number that > 2**32 */
  else
    res.origin_server_ts = 0; /* 1970... */

  res.content = cJSON_GetObjectItemCaseSensitive(res.raw_json, "content");

  return(res);
}
MatrixEventMessage matrixbot_message_from(cJSON *json) {
  MatrixEventMessage msg;
  cJSON *jbuff;

  MATRIXBOT_SETPROPU(msg, json, jbuff, msgtype);
  MATRIXBOT_SETPROPU(msg, json, jbuff, body);

  return(msg);
}
MatrixEventMessageEdit matrixbot_messageedit_from(cJSON *json) {
  MatrixEventMessageEdit msg;
  cJSON *jbuff;

  jbuff = cJSON_GetObjectItemCaseSensitive(json, "m.relates_to");
  if (jbuff) {
    jbuff = cJSON_GetObjectItemCaseSensitive(json, "event_id");
    if (jbuff) {
      msg.changed_event_id = jbuff->valuestring;
    }
  } else {
    msg.changed_event_id = 0;
  }
  msg.message = matrixbot_message_from(json);

  return(msg);
}

MatrixBot matrixbot_new(const char *homeserver, const char *access_token) {
  MatrixBot bot;

  memset(&bot, 0, sizeof(bot));
  strncpy(bot.client.homeserver, homeserver, sizeof(bot.client.homeserver));
  strncpy(bot.client.access_token, access_token, sizeof(bot.client.access_token));

  return(bot);
}

__attribute__((deprecated("Use cJSON (or libjson-c) functions")))
void* matrixbot_json_getpath(void* json, ...) {
  return(0);
}

MatrixBotLoopResult matrixbot_loop(MatrixBot *bot) {
  cJSON *root, *rooms, *event, *events, *timeline;
  char *room_id;
  size_t timeline_len, idx;
  MatrixBotLoopResult result;

  result.errcode[0] = 0;
  result.error[0] = 0;
  root = matrix_sync(&bot->client, 0);
  //rooms = JSON(root, "rooms", "join", 0);
  //if (!rooms || !json_object_is_type(rooms, json_type_object)) {
  if (!(rooms = cJSON_GetObjectItemCaseSensitive(root, "rooms")) 
      || !(rooms = cJSON_GetObjectItemCaseSensitive(rooms, "join")) 
      || !cJSON_IsObject(rooms)) {
    strncpy(result.errcode, cJSON_GetObjectItemCaseSensitive(root, "errcode")->valuestring, sizeof(result.errcode));
    strncpy(result.error, cJSON_GetObjectItemCaseSensitive(root, "error")->valuestring, sizeof(result.error));
    cJSON_Delete(root);
    return result;
  }

  /* not ansi c, but this bot uses c99... */
  for (events = rooms->child; events != 0; events = events->next) {
    room_id = events->string;
    if (!(timeline = cJSON_GetObjectItemCaseSensitive(events, "timeline"))
        || !(timeline = cJSON_GetObjectItemCaseSensitive(timeline, "events"))
        || !cJSON_IsArray(timeline))
      continue;

    timeline_len = cJSON_GetArraySize(timeline);
    for (idx = 0; idx < timeline_len; idx++) {
      MatrixEvent matrix_event;
      MatrixBotContext ctx;

      event = cJSON_GetArrayItem(timeline, idx);
      matrix_event = matrixbot_event_from(event, room_id);
      ctx.client = bot;
      ctx.event = &matrix_event;
      if (matrix_event.content && strcmp("m.room.message", matrix_event.type) == 0) {
        if (bot->handlers.message_edit && cJSON_GetObjectItemCaseSensitive(matrix_event.content, "m.new_content")) {
          MatrixEventMessageEdit msg;
          msg = matrixbot_messageedit_from(matrix_event.content);

          bot->handlers.message_edit(ctx, &msg);
        } else if (bot->handlers.message_new) {
          MatrixEventMessage msg;
          msg = matrixbot_message_from(matrix_event.content);

          bot->handlers.message_new(ctx, &msg);
        }
      } else if (bot->handlers.message_redact && strcmp("m.room.redaction", matrix_event.type) == 0) {
        const char *redacted_because;
        cJSON *reason;

        reason = cJSON_GetObjectItemCaseSensitive(matrix_event.content, "reason");
        if (reason) {
          redacted_because = reason->valuestring;
        } else {
          redacted_because = 0;
        }

        bot->handlers.message_redact(ctx, redacted_because);
      }
    }
  }
  cJSON_Delete(root);

  return result;
}

MatrixSendResult matrixbot_send(MatrixBotContext ctx, const char *message) {
  MatrixSendResult res;
  cJSON *json;
  char *jsonstr;
  json = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "msgtype", cJSON_CreateString("m.text"));
  cJSON_AddItemToObject(json, "body", cJSON_CreateString(message));

  jsonstr = cJSON_Print(json);
  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", jsonstr);

  free(jsonstr);
  cJSON_Delete(json);

  return(res);
}
MatrixSendResult matrixbot_reply(MatrixBotContext ctx, const char *message) {
  MatrixSendResult res;
  cJSON *json, *mrelto, *minreplto;
  char *jsonstr;

  json = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "msgtype", cJSON_CreateString("m.text"));
  cJSON_AddItemToObject(json, "body", cJSON_CreateString(message));
  minreplto = cJSON_CreateObject();
  cJSON_AddItemToObject(minreplto, "event_id", cJSON_CreateString(ctx.event->event_id));
  mrelto = cJSON_CreateObject();
  cJSON_AddItemToObject(mrelto, "m.in_reply_to", minreplto);
  cJSON_AddItemToObject(json, "m.relates_to", mrelto);

  jsonstr = cJSON_Print(json);
  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", jsonstr);

  free(jsonstr);
  cJSON_Delete(json);

  return(res);
}
MatrixSendResult matrixbot_sendf(MatrixBotContext ctx, const char *fmt, ...) {
  MatrixSendResult res;
  cJSON *json;
  char message[MATRIXBOT_VSEND_MSGSIZE], *jsonstr;
  va_list va;

  va_start(va, fmt);
  vsnprintf(message, sizeof(message), fmt, va);
  va_end(va);

  json = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "msgtype", cJSON_CreateString("m.text"));
  cJSON_AddItemToObject(json, "body", cJSON_CreateString(message));

  jsonstr = cJSON_Print(json);
  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", jsonstr);

  free(jsonstr);
  cJSON_Delete(json);

  return(res);
}
MatrixSendResult matrixbot_replyf(MatrixBotContext ctx, const char *fmt, ...) {
  MatrixSendResult res;
  cJSON *json, *mrelto, *minreplto;
  char message[MATRIXBOT_VSEND_MSGSIZE], *jsonstr;
  va_list va;

  va_start(va, fmt);
  vsnprintf(message, sizeof(message), fmt, va);
  va_end(va);

  json = cJSON_CreateObject();
  cJSON_AddItemToObject(json, "msgtype", cJSON_CreateString("m.text"));
  cJSON_AddItemToObject(json, "body", cJSON_CreateString(message));
  minreplto = cJSON_CreateObject();
  cJSON_AddItemToObject(minreplto, "event_id", cJSON_CreateString(ctx.event->event_id));
  mrelto = cJSON_CreateObject();
  cJSON_AddItemToObject(mrelto, "m.in_reply_to", minreplto);
  cJSON_AddItemToObject(json, "m.relates_to", mrelto);

  jsonstr = cJSON_Print(json);
  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", jsonstr);

  free(jsonstr);
  cJSON_Delete(json);

  return(res);
}
