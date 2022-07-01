/* sorry for this code
 * may be it will be rewritten (rm -rf)
 */

#include <stdio.h>
#include <stdlib.h>

#include "include/matrix-autocmd.h"

struct MXAutoCMDHandlerNode *MXAUTOCMD_FUNCTIONS = 0;

void mxautocmd_register(
    const char *name,
    enum MXAutoCMDType type,
    void (*handler)(MatrixBotContext ctx, ...)
  ) {
  struct MXAutoCMDHandlerNode *node;

  if (MXAUTOCMD_FUNCTIONS == 0) {
    MXAUTOCMD_FUNCTIONS = malloc(sizeof(struct MXAutoCMDHandlerNode));
    node = MXAUTOCMD_FUNCTIONS;
  } else {
    for (node = MXAUTOCMD_FUNCTIONS; node->next != 0; node = node->next);
    node->next = malloc(sizeof(struct MXAutoCMDHandlerNode));
    node = node->next;
  }

  node->name = name;
  node->type = type;
  node->handler = handler;
  node->next = 0;
}

