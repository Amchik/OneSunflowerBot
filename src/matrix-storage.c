#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cjson/cJSON.h>

#include "include/matrix-storage.h"

__attribute__((nonnull(1)))
MatrixStorage matrixstorage_create(const char *filename) {
  MatrixStorage res;
  char *jsonstr;
  size_t flen;
  cJSON *root, *obj;
  FILE *fp;

  res.filename = filename;
  fp = fopen(filename, "r");
  if (!fp)
    return(res);

  fseek(fp, 0, SEEK_END);
  flen = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  if (flen == 0) {
    fclose(fp);
    return(res);
  }
  jsonstr = malloc(flen + 1);
  jsonstr[flen] = 0;
  fread(jsonstr, 1, flen, fp);
  fclose(fp);
  root = cJSON_Parse(jsonstr);
  free(jsonstr);

  for (obj = root->child; obj != 0; obj = obj->next) {
    /* will copy */
    matrixstorage_append(&res, obj->string, obj->valuestring);
  }
  cJSON_Delete(root);

  return(res);
}
int matrixstorage_save(MatrixStorage self) {
  MatrixStorageNode *node;
  char *jsonstr;
  size_t len;
  cJSON *json;
  FILE *fp;

  fp = fopen(self.filename, "w");
  if (!fp) {
    return(errno);
  }

  json = cJSON_CreateObject();
  for (node = self.node; node != 0; node = node->next) {
    cJSON_AddStringToObject(json, node->name, node->value);
  }
  jsonstr = cJSON_PrintUnformatted(json);
  cJSON_Delete(json);

  len = strlen(jsonstr);
  fseek(fp, 0, SEEK_SET);
  fwrite(jsonstr, 1, len, fp);
  fclose(fp);
  free(jsonstr);

  return(0);
}
void matrixstorage_close(MatrixStorage self) {
  MatrixStorageNode *node, *next;

  for (node = self.node; node != 0; node = next) {
    next = node->next;
    free(node);
  }
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
