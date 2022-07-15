#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../include/matrix-autocmd.h"

void cmd_ping(MatrixBotContext ctx, MatrixEventMessage *msg) {
  unsigned long tm;
  struct timeval t;

  gettimeofday(&t, 0);
  tm = (t.tv_sec * 1000000 + t.tv_usec) / 1000;

  matrixbot_qreplyf(ctx, "Ping-pong! 🏓 %lums",
      tm - ctx.event->origin_server_ts);
  printf("\033[1;34minfo:\033[0m Ping: %lums\n", tm - ctx.event->origin_server_ts);
}
void cmd_score(MatrixBotContext ctx, MatrixEventMessage *msg) {
  MatrixStorageNode *node;
  int score;
  char nodename[128];

  snprintf(nodename, sizeof(nodename), "misc.score.%s", ctx.event->sender);
  node = matrixstorage_find(ctx.client->storage, nodename);
  if (!node) {
    score = 0;
    node = matrixstorage_append(&ctx.client->storage, nodename, "0");
  } else {
    score = atoi(node->value);
    score += rand() % 5;
    snprintf(node->value, sizeof(node->value), "%d", score);
  }
  matrixstorage_save(ctx.client->storage);

  matrixbot_qreplyf(ctx, "You score is %d! Type this command again to up it", score);
}
void cmd_batch(MatrixBotContext ctx, MatrixEventMessage *msg) {
  matrixbot_qreplyf(ctx, "%s", ctx.client->client.batch);
}

mxmod {
  mxautocmd_register("!ping", MXAUTOCMD_COMMAND, (void (*)(MatrixBotContext ctx, ...))cmd_ping);
  mxautocmd_register("!score", MXAUTOCMD_COMMAND, (void (*)(MatrixBotContext ctx, ...))cmd_score);
  mxautocmd_register("!batch", MXAUTOCMD_COMMAND, (void (*)(MatrixBotContext ctx, ...))cmd_batch);
}

