/* vim: ft=c
 *
 * matrix-defines.h
 */

/* note: some hs can uses access_tokens that > 128 */
#define MATRIX_ACCESSTOKEN_LEN 128

#define MATRIXNODE_FREENODE   1
#define MATRIXNODE_FREEKEY    2
#define MATRIXNODE_FREEVALUE  4
#define MATRIXNODE_AUTOFREE   8 /* doesn't works */

