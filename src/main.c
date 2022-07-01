#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <signal.h>

#include "include/matrix-bot.h"
#include "include/matrix-autocmd.h"
#include "include/matrix-client.h"

#ifndef MATRIXBOT_DEFAULTHS
#define MATRIXBOT_DEFAULTHS "matrix-client.matrix.org"
#endif

void on_message(MatrixBotContext ctx, MatrixEventMessage *msg) {
  struct MXAutoCMDHandlerNode *node;
  size_t i, len;

  if (!msg->body) return;
  for (node = MXAUTOCMD_FUNCTIONS; node != 0; node = node->next) {
    switch (node->type) {
      case MXAUTOCMD_COMMAND:
        len = strlen(node->name);
        for (i = 0; i < len; i++)
          if (node->name[i] != msg->body[i])
            break;
        if (msg->body[len] != 0 && msg->body[len] != ' ')
          break;

      case MXAUTOCMD_MESSAGE:
        node->handler(ctx, msg);
        break;

      case MXAUTOCMD_EDIT:
      case MXAUTOCMD_REDACT:
        break;
    }
  }
}

void get_token(char *filepath, char token[MATRIX_ACCESSTOKEN_LEN]) {
  FILE *fp;
  size_t nread;
  char *path;

  path = filepath ? filepath : "./.matrix_token";

  fp = fopen(path, "r");
  if (!fp) {
    fprintf(stderr, "\033[1;31merror:\033[0m failed to read token from file '%s': (#%d) %s\n"
        "\033[1mnote:\033[0m you can set token using --token key or create file named '.matrix_token'\n",
        path, errno, strerror(errno));
    exit(1);
  }
  nread = fread(token, sizeof(char), MATRIX_ACCESSTOKEN_LEN, fp);
  if (nread == (size_t)-1) {
    fprintf(stderr, "\033[1;31merror:\033[0m failed to read token from file '%s': (#%d) %s\n",
        path, errno, strerror(errno));
    exit(1);
  } else if (nread == 0) {
    fprintf(stderr, "\033[1;31merror:\033[0m failed to read token from file '%s': file is empty\n",
        path);
    exit(1);
  }
  token[nread == MATRIX_ACCESSTOKEN_LEN ? nread - 1 : nread] = 0;
  if (token[nread == MATRIX_ACCESSTOKEN_LEN ? nread - 2 : nread - 1] == '\n') {
    token[nread - 1] = 0;
  }

  fclose(fp);
}

__attribute__((noreturn))
void on_signal(int sig) {
  if (sig == SIGINT || sig == SIGTERM) {
    printf("\r\033[1;34minfo:\033[0;1m stopping bot...\033[0m\n");
  } else {
    fprintf(stderr, "\033[1;31mfatal:\033[0;1m main thread terminated by signal %d (%s)\033[0m\n",
        sig, strsignal(sig));
    exit(128 + sig);
  }
  exit(0);
}

int main(int argc, char **argv) {
  MatrixBot bot;
  int i;
  char has_token, failed_times;

  for (i = 0; i < 31; i++) {
    signal(i, on_signal);
  }

  srand(time(0));

  bot = matrixbot_new(MATRIXBOT_DEFAULTHS, "");
  has_token = 0;

  for (i = 1; i < argc; ++i) {
#define arg(long, short) ( (long && !strcmp(long, argv[i])) || (short && !strcmp(short, argv[i])) )
    if arg("--tokenfile", "-f") {
      if (i++ >= argc)
        break;

      get_token(argv[i], bot.client.access_token);
      has_token = 1;
    } else if arg("--token", "-t") {
      if (i++ >= argc)
        break;

      strncpy(bot.client.access_token, argv[i], sizeof(bot.client.access_token));
      has_token = 1;
    } else if arg("--homeserver", "-h") {
      if (i++ >= argc)
        break;

      strncpy(bot.client.homeserver, argv[i], sizeof(bot.client.homeserver));
    }
#undef arg
  }
  if (!has_token) {
    get_token(0, bot.client.access_token);
  }

  bot.handlers.message_new = on_message;

  printf("\033[1;34minfo:\033[0;1m bot starting...\033[0m\n"
      "  >> homeserver: %s\n", bot.client.homeserver);

  matrixbot_qsync(bot);

  failed_times = 0;
  printf("\033[1;34minfo:\033[0;1m bot started\033[0m\n");
  while (1) {
    MatrixBotLoopResult res;

    res = matrixbot_loop(&bot);
    if (*res.error != 0) {
      failed_times += 1;
      printf("\033[1;33mwarn:\033[0;1m sync request failed\033[0m\n"
          "  >> errcode: %s\n"
          "  >> error:   %s\n",
          res.errcode, res.error);
      if (failed_times >= 5) {
        printf("\033[1;31merror:\033[0;1m sync request failed 5 times\033[0m\n");
        raise(SIGABRT);
      }
    } else {
      failed_times = 0;
    }

    usleep(50000);
  }

  return(0);
}
