/* vim: ft=c
 *
 * matrix-autocmd.h
 * - part of onesunflowerbot
 *
 * id: TAIRV404
 */

/* sorry for this code */

#ifndef __TAIRV404_matrix_autocmd
#define __TAIRV404_matrix_autocmd

#include "matrix-bot.h"

enum MXAutoCMDType {
  MXAUTOCMD_COMMAND,
  MXAUTOCMD_MESSAGE,
  MXAUTOCMD_EDIT,
  MXAUTOCMD_REDACT
};

struct MXAutoCMDHandlerNode {
  const char *name;
  enum MXAutoCMDType type;
  void (*handler)(MatrixBotContext ctx, ...);
  struct MXAutoCMDHandlerNode *next;
};

extern struct MXAutoCMDHandlerNode *MXAUTOCMD_FUNCTIONS;

/**
 * Register command handler.
 */
void mxautocmd_register(const char *name, enum MXAutoCMDType type, void (*handler)(MatrixBotContext ctx, ...));

#define mxmod __attribute__((constructor)) static void _mxautocmd_mxmod_begin(void)

#endif
