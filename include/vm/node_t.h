/** -------------------------------------------
 * @file   node_t.h
 * @brief  Emfrp Node Definition(IR)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/11
 ------------------------------------------- */
#pragma once
#include "ast.h"
#include "em_result.h"
#include "vm/object_t.h"

// ! Program kind of node.
typedef enum node_program_kind {
  // ! no program.(it may be updated via emfrp_indicate_node_update.)
  NODE_PROGRAM_KIND_NOTHING,
  // ! containing AST.
  NODE_PROGRAM_KIND_AST,
  // ! containing callback.
  NODE_PROGRAM_KIND_CALLBACK
} node_program_kind;

typedef object_t * (*node_callback_t)(void);

// ! Node definition struct.
typedef struct node_t {
  // ! Name of node.
  string_t name;
  // ! Kind of node_t::program.
  node_program_kind program_kind;
  // ! Program
  union {
    // ! Nothing
    nullptr_t nothing;
    // ! AST
    parser_expression_t * ast;
    // ! CallBack
    node_callback_t callback;
  } program;
  object_t * value;
  // ! The action when the value is changed.
  void (*action)(object_t *);
} node_t;

// ! Construct node_t without any programs.
/* !
 * \param out The result
 * \param name The name
 * \return The status code
 */
static inline em_result node_new_nothing(node_t * out, string_t name) {
  out->name = name;
  out->program_kind = NODE_PROGRAM_KIND_NOTHING;
  out->program.nothing = nullptr;
  out->value = nullptr;
  out->action = nullptr;
  return EM_RESULT_OK;
}

// ! Construct node_t from AST.
/* !
 * \param out The result
 * \param name The name
 * \param ast The program
 * \return The status code
 */
em_result node_new_ast(node_t * out, string_t name, parser_expression_t * ast); 

// ! Construct node_t from callback.
/* !
 * \param out The result
 * \param name The name
 * \param callback The program
 * \return The status code
 */
em_result node_new_callback(node_t * out, string_t name, node_callback_t callback); 

// ! Freeing Deeply node_t
/* !
 * \param v The node to be freed
 */
void node_deep_free(node_t * v);
