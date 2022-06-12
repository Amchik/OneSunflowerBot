#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <json-c/json.h>

#include "include/matrix-defines.h"
#include "include/matrix-client.h"
#include "include/matrix-bot.h"

#define MATRIXBOT_SETPROPU(res, orjson, json, prop)\
  json = json_object_object_get(orjson, #prop);\
  if (json)\
    res.prop = json_object_get_string(json);\
  else\
    res.prop = 0
#define MATRIXBOT_SETPROP(res, json, prop) MATRIXBOT_SETPROPU(res, res.raw_json, json, prop)

MatrixEvent matrixbot_event_from(json_object *json, const char *room_id) {
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
  json = json_object_object_get(res.raw_json, "origin_server_ts");
  if (json)
    res.origin_server_ts = (u_int64_t)json_object_get_int64(json);
  else
    res.origin_server_ts = 0; /* 1970... */

  res.content = json_object_object_get(res.raw_json, "content");

  return(res);
}
MatrixEventMessage matrixbot_message_from(json_object *json) {
  MatrixEventMessage msg;
  json_object *jbuff;

  MATRIXBOT_SETPROPU(msg, json, jbuff, msgtype);
  MATRIXBOT_SETPROPU(msg, json, jbuff, body);

  return(msg);
}
MatrixEventMessageEdit matrixbot_messageedit_from(json_object *json) {
  MatrixEventMessageEdit msg;
  json_object *jbuff;

  jbuff = json_object_object_get(json, "m.relates_to");
  if (jbuff) {
    jbuff = json_object_object_get(json, "event_id");
    if (jbuff) {
      msg.changed_event_id = json_object_get_string(jbuff);
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

#define JSON matrixbot_json_getpath
json_object* matrixbot_json_getpath(json_object* json, ...) {
  char *buff;
  va_list va;
  
  va_start(va, json);

  for (buff = va_arg(va, char*); buff != 0; buff = va_arg(va, char*)) {
    json = json_object_object_get(json, buff);
    if (!json) return(json);
  }

  va_end(va);

  return(json);
}

void matrixbot_loop(MatrixBot *bot) {
  json_object *root, *rooms, *event;
  size_t events_len, idx;

  root = matrix_sync(&bot->client, 0);
  rooms = JSON(root, "rooms", "join", 0);
  if (!rooms || !json_object_is_type(rooms, json_type_object)) {
    json_object_put(root);
    return; /* to-do: error handling */
  }

  /* not ansi c, but this bot uses c99... */
  json_object_object_foreach(rooms, room_id, events) {
    events = JSON(events, "timeline", "events", 0);
    if (!events || !json_object_is_type(events, json_type_array))
      continue;

    events_len = json_object_array_length(events);
    for (idx = 0; idx < events_len; idx++) {
      MatrixEvent matrix_event;
      MatrixBotContext ctx;

      event = json_object_array_get_idx(events, idx);
      matrix_event = matrixbot_event_from(event, room_id);
      ctx.client = bot;
      ctx.event = &matrix_event;
      if (matrix_event.content && strcmp("m.room.message", matrix_event.type) == 0) {
        if (bot->handlers.message_edit && json_object_object_get(matrix_event.content, "m.new_content")) {
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
        json_object *reason;

        reason = json_object_object_get(matrix_event.content, "reason");
        if (reason) {
          redacted_because = json_object_get_string(reason);
        } else {
          redacted_because = 0;
        }

        bot->handlers.message_redact(ctx, redacted_because);
      }
    }
  }
  json_object_put(root);
}

MatrixSendResult matrixbot_send(MatrixBotContext ctx, const char *message) {
  MatrixSendResult res;
  json_object *json;
  json = json_object_new_object();
  json_object_object_add(json, "msgtype", json_object_new_string("m.text"));
  json_object_object_add(json, "body", json_object_new_string(message));

  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", (char*)json_object_to_json_string(json));

  json_object_put(json);

  return(res);
}
MatrixSendResult matrixbot_reply(MatrixBotContext ctx, const char *message) {
  MatrixSendResult res;
  json_object *json, *mrelto, *minreplto;

  json = json_object_new_object();
  json_object_object_add(json, "msgtype", json_object_new_string("m.text"));
  json_object_object_add(json, "body", json_object_new_string(message));
  minreplto = json_object_new_object();
  json_object_object_add(minreplto, "event_id", json_object_new_string(ctx.event->event_id));
  mrelto = json_object_new_object();
  json_object_object_add(mrelto, "m.in_reply_to", minreplto);
  json_object_object_add(json, "m.relates_to", mrelto);

  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", (char*)json_object_to_json_string(json));

  json_object_put(json);

  return(res);
}
MatrixSendResult matrixbot_sendf(MatrixBotContext ctx, const char *fmt, ...) {
  MatrixSendResult res;
  json_object *json;
  char message[MATRIXBOT_VSEND_MSGSIZE];
  va_list va;

  va_start(va, fmt);
  vsnprintf(message, sizeof(message), fmt, va);
  va_end(va);

  json = json_object_new_object();
  json_object_object_add(json, "msgtype", json_object_new_string("m.text"));
  json_object_object_add(json, "body", json_object_new_string(message));

  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", (char*)json_object_to_json_string(json));

  json_object_put(json);

  return(res);
}
MatrixSendResult matrixbot_replyf(MatrixBotContext ctx, const char *fmt, ...) {
  MatrixSendResult res;
  json_object *json, *mrelto, *minreplto;
  char message[MATRIXBOT_VSEND_MSGSIZE];
  va_list va;

  va_start(va, fmt);
  vsnprintf(message, sizeof(message), fmt, va);
  va_end(va);

  json = json_object_new_object();
  json_object_object_add(json, "msgtype", json_object_new_string("m.text"));
  json_object_object_add(json, "body", json_object_new_string(message));
  minreplto = json_object_new_object();
  json_object_object_add(minreplto, "event_id", json_object_new_string(ctx.event->event_id));
  mrelto = json_object_new_object();
  json_object_object_add(mrelto, "m.in_reply_to", minreplto);
  json_object_object_add(json, "m.relates_to", mrelto);

  res = matrix_send(&ctx.client->client, ctx.event->room_id, "m.room.message", (char*)json_object_to_json_string(json));

  json_object_put(json);

  return(res);
}
