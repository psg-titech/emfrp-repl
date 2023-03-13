/** -------------------------------------------
 * @file   node_t.c
 * @brief  Emfrp Node Implementation(IR)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/19
 ------------------------------------------- */

#include "vm/node_t.h"

void
node_deep_free(node_t * v) {
  string_free(&(v->name));
}
