/* vim: ft=c
 *
 * matrix-storage.h
 * - part of onesunflowerbot
 *
 * id: DpKHnE0H
 */

#ifndef __DpKHnE0H_matrix_storage_h
#define __DpKHnE0H_matrix_storage_h

#include <stdio.h>

typedef struct MatrixStorageNode {
  /**
   * Name of storage unit. Cannot be NULL.
   */
  char name[128];
  /**
   * Value of storage unit. Cannot be NULL.
   */
  char value[256];

  /**
   * Pointer to next node. Can be NULL if this is last node.
   */
  struct MatrixStorageNode *next;
} MatrixStorageNode;
typedef struct {
  /**
   * Path to storage.
   */
  const char *filename;
  /**
   * Pointer to first node. Can be NULL if
   * no nodes in storage.
   */
  MatrixStorageNode *node;

  /**
   * File stream. Open in 'w' mode, but sometimes
   * can be NULL if failed to open it.
   */
  FILE *_fileptr;
} MatrixStorage;

/**
 * Creates (allocates) a MatrixStorage in filename.
 */
__attribute__((nonnull(1)))
MatrixStorage matrixstorage_create(const char *filename);
/**
 * Save data to filename.
 */
void matrixstorage_save(MatrixStorage self);
/**
 * Close MatrixStorage. It will deallocate nodes and
 * close file.
 */
void matrixstorage_close(MatrixStorage self);

/**
 * Append value to MatrixStorage.
 * Resulted node is mutable and can be saved by matrixstorage_save().
 */
__attribute__((nonnull(2, 3)))
MatrixStorageNode* matrixstorage_append(MatrixStorage *self, const char *name, const char *value);

/**
 * Find node in MatrixStorage. If node does not exists will return NULL.
 * Resulted node is mutable and can be saved by matrixstorage_save().
 */
__attribute__((nonnull(2)))
MatrixStorageNode* matrixstorage_find(MatrixStorage self, const char *name);

#endif

