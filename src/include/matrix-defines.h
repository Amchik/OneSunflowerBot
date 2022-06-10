/* vim: ft=c
 *
 * matrix-defines.h
 */

/* note: some hs can uses access_tokens that > 128 */
#define MATRIX_ACCESSTOKEN_LEN 128

#define MATRIXNODE_FREENODE   0b0001
#define MATRIXNODE_FREEKEY    0b0010
#define MATRIXNODE_FREEVALUE  0b0100
#define MATRIXNODE_AUTOFREE   0b1000 /* doesn't works */

