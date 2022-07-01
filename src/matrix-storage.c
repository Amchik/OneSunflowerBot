#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

#include "include/matrix-storage.h"

__attribute__((nonnull(1)))
MatrixStorage matrixstorage_create(const char *filename) {
  MatrixStorage res;
  char *jsonstr;
  size_t flen;
  cJSON *root, *obj;

  res.filename = filename;
  res._fileptr = fopen(filename, "r");
  if (!res._fileptr)
    goto open_write;

  fseek(res._fileptr, 0, SEEK_END);
  flen = ftell(res._fileptr);
  fseek(res._fileptr, 0, SEEK_SET);
  if (flen == 0) {
    fclose(res._fileptr);
    goto open_write;
  }
  jsonstr = malloc(flen + 1);
  jsonstr[flen] = 0;
  fread(jsonstr, 1, flen, res._fileptr);
  fclose(res._fileptr);
  root = cJSON_ParseWithLength(jsonstr, flen);
  free(jsonstr);

  for (obj = root->child; obj != 0; obj = obj->next) {
    /* will copy */
    matrixstorage_append(&res, obj->string, obj->valuestring);
  }
  cJSON_Delete(root);

open_write:
  res._fileptr = fopen(filename, "w");
  if (!res._fileptr) {
    res.node = 0;
  }

  return(res);
}
void matrixstorage_save(MatrixStorage self) {
  MatrixStorageNode *node;
  char *jsonstr;
  size_t len;
  cJSON *json;

  json = cJSON_CreateObject();
  for (node = self.node; node != 0; node = node->next) {
    cJSON_AddStringToObject(json, node->name, node->value);
  }
  jsonstr = cJSON_PrintUnformatted(json);
  cJSON_Delete(json);

  len = strlen(jsonstr);
  fseek(self._fileptr, 0, SEEK_SET);
  fwrite(jsonstr, 1, len, self._fileptr);
  free(jsonstr);
}
void matrixstorage_close(MatrixStorage self) {
  MatrixStorageNode *node, *next;

  for (node = self.node; node != 0; node = next) {
    next = node->next;
    free(node);
  }
  fclose(self._fileptr);
}

__attribute__((nonnull(2, 3)))
MatrixStorageNode* matrixstorage_append(MatrixStorage *self, const char *name, const char *value) {
  MatrixStorageNode *node;

  if (!self->node) {
    self->node = malloc(sizeof(*self->node));
    node = self->node;
  } else {
    for (node = self->node; node->next != 0; node = node->next);
    node->next = malloc(sizeof(*node->next));
    node = node->next;
  }

  strncpy(node->name, name, sizeof(node->name));
  strncpy(node->value, value, sizeof(node->value));
  node->next = 0;

  return(node);
}

__attribute__((nonnull(2)))
MatrixStorageNode* matrixstorage_find(MatrixStorage self, const char *name) {
  MatrixStorageNode *node;

  for (node = self.node; node != 0; node = node->next)
    if (!strcmp(node->name, name))
      break;

  return(node);
}
