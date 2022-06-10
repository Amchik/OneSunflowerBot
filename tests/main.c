#include <wchar.h>

#include "include/matrix-client.h"
#include "include/utests.h"

#define require(name) extern char* name();

require(test_matrix_newnode)
require(test_matrixnode_stringify)

int utests() {
  utest_t *chain;

  chain = utests_alloc("Test matrix_newnode()", test_matrix_newnode,
      utests_alloc("Test matrixnode_stringify()", test_matrixnode_stringify, 0));

  utests_run(chain);
}
