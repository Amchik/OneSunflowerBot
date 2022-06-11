#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "include/matrix-client.h"
#include "include/utests.h"

static char _BUFFER[256];

char* test_matrix_newnode() {
  MatrixNode origin;
  char *key;
  wchar_t *value;

  key = "foo";
  value = L"bar";

  origin = matrix_newnode("origin_key", L"origin_value", 0,
      &matrix_newnode(key, value, 0,
        &matrix_newnode("FALSE", L"FALSE", 0, 0)));

  utests_assert(origin.next->key == key);
  utests_assert(origin.next->value == value);
  utests_assert(origin.next->flags == 0);

  return(0);
}

char* test_matrixnode_stringify() {
  MatrixNode chain;
  char *expected, *actual;
  size_t result;

  expected = "{ \"field1_big_0cf9180a764aba863a67b6d72f0918bc131c6772642cb2dce5a34f0a702f9470ddc2bf125c12198b1995c233c34b4afd346c54a2334c350a948a51b6e8b4e6b6\": \"Hello, world!, 0cf9180a764aba863a67b6d72f0918bc131c6772642cb2dce5a34f0a702f9470ddc2bf125c12198b1995c233c34b4afd346c54a2334c350a948a51b6e8b4e6b6\", \"field2\": \"42\", \"field3\": \"\" }";
  chain = matrix_newnode("field1_big_0cf9180a764aba863a67b6d72f0918bc131c6772642cb2dce5a34f0a702f9470ddc2bf125c12198b1995c233c34b4afd346c54a2334c350a948a51b6e8b4e6b6", L"Hello, world!, 0cf9180a764aba863a67b6d72f0918bc131c6772642cb2dce5a34f0a702f9470ddc2bf125c12198b1995c233c34b4afd346c54a2334c350a948a51b6e8b4e6b6", 0,
      &matrix_newnode("field2", L"42", 0,
        &matrix_newnode("field3", L"", 0, 0)));

  actual = matrixnode_stringify(chain);
  
  result = strcmp(expected, actual);
  free(actual);
  if (result != 0) {
    sprintf(_BUFFER, "strlen(expected) - strlen(actual) = %lu (expected 0)", result);
    return(_BUFFER);
  }

  return(0);
}

