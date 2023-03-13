/** -------------------------------------------
 * @file   node_t.h
 * @brief  Emfrp Node Definition(IR)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/11
 ------------------------------------------- */
#pragma once
#include "ast.h"
#include "em_result.h"
#include "vm/object_t.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef void (*node_event_delegate_t)(object_t *);

// ! Node definition struct.
typedef struct node_t {
  // ! Name of node.
  string_t name;
  // ! Value of node.
  object_t * value;
  // ! Value of node@lst.
  object_t * last;
  // ! The action when the value is changed.
  node_event_delegate_t action;
} node_t;

// ! Construct node_t without any programs.
/* !
 * \param out The result
 * \param name The name
 * \return The status code
 */
static inline em_result
node_new(node_t * out, string_t name) {
  out->name = name;
  out->value = nullptr;
  out->last = nullptr;
  out->action = nullptr;
  return EM_RESULT_OK;
}

// ! Freeing Deeply node_t
/* !
 * \param v The node to be freed
 */
void node_deep_free(node_t * v);

#ifdef __cplusplus
}
#endif /* __cplusplus */
