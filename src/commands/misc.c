#include <stdio.h>
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

mxmod {
  mxautocmd_register("!ping", MXAUTOCMD_COMMAND, (void*)cmd_ping);
}

