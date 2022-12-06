/** -------------------------------------------
 * @file   exec_sequence_t.h
 * @brief  Emfrp Execution Sequence Type
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/6
 ------------------------------------------- */
#pragma once
#include "em_result.h"
#include "ast.h"
#include "collections/arraylist_t.h"
#include "vm/node_t.h"

typedef object_t * (*exec_callback_t)(void);
struct machine_t;

typedef enum node_or_tuple_t_kind {
  NODE_OR_TUPLE_NONE,
  NODE_OR_TUPLE_NODE,
  NODE_OR_TUPLE_TUPLE
} node_or_tuple_t_kind;

// ! type node_or_tuple = Node of Node | Tuple of arrayList<node_or_tuple>
typedef struct node_or_tuple_t {
  node_or_tuple_t_kind kind;
  union{
    node_t * node;
    arraylist_t /*<node_or_tuple_t>*/ tuple;
  } value;
} node_or_tuple_t;

// ! Program kind of node.
typedef enum exec_sequence_program_kind {
  // ! no program.(it may be updated via emfrp_indicate_node_update.)
  EXEC_SEQUENCE_PROGRAM_KIND_NOTHING,
  // ! containing AST.
  EXEC_SEQUENCE_PROGRAM_KIND_AST,
  // ! containing callback.
  EXEC_SEQUENCE_PROGRAM_KIND_CALLBACK
} exec_sequence_program_kind;

typedef struct exec_sequence_t {
  // ! Kind of exec_sequence_t::program.
  exec_sequence_program_kind program_kind;
  // ! Program
  union {
    // ! Nothing
    nullptr_t nothing;
    // ! AST
    parser_expression_t * ast;
    // ! CallBack
    exec_callback_t callback;
  } program;
  // Mono node definition or 'as' defined node. Nullable.
  node_t * node_definition;
  // Multiple node definitions. Nullable.
  node_or_tuple_t * /*<node_t *>*/ node_definitions;
} exec_sequence_t;

// ! Constructor of exec_sequence_t.
/* !
 * \param out The result
 * \param value The node to update.
 */
static inline em_result
exec_sequence_new_mono_nothing(exec_sequence_t * out, node_t * value) {
  out->program_kind = EXEC_SEQUENCE_PROGRAM_KIND_NOTHING;
  out->program.nothing = nullptr;
  out->node_definition = value;
  out->node_definitions = nullptr;
  return EM_RESULT_OK;
}

// ! Constructor of exec_sequence_t.
/* !
 * \param out The result
 * \param ast The program(AST)
 * \param value The node to update.
 */
static inline em_result
exec_sequence_new_mono_ast(exec_sequence_t * out, parser_expression_t * ast, node_t * value) {
  out->program_kind = EXEC_SEQUENCE_PROGRAM_KIND_AST;
  out->program.ast = ast;
  out->node_definition = value;
  out->node_definitions = nullptr;
  return EM_RESULT_OK;
}

// ! Constructor of exec_sequence_t.
/* !
 * \param out The result
 * \param callback The program(callback)
 * \param value The node to update.
 * \param The status result
 */
static inline em_result
exec_sequence_new_mono_callback(exec_sequence_t * out, exec_callback_t callback, node_t * value) {
  out->program_kind = EXEC_SEQUENCE_PROGRAM_KIND_CALLBACK;
  out->program.callback = callback;
  out->node_definition = value;
  out->node_definitions = nullptr;
  return EM_RESULT_OK;
}

// ! Constructor of exec_sequence_t.
/* !
 * \param out The result
 * \param callback The program(callback)
 * \param as_value The node (`as` declaration) to update.
 * \param value The nodes to update.
 * \param The status result
 */
static inline em_result
exec_sequence_new_multi_callback(exec_sequence_t * out, exec_callback_t callback, node_t * as_value, node_or_tuple_t * value) {
  out->program_kind = EXEC_SEQUENCE_PROGRAM_KIND_CALLBACK;
  out->program.callback = callback;
  out->node_definition = as_value;
  out->node_definitions = value;
  return EM_RESULT_OK;
}

// ! Update the value of nodes.
/* !
 * \param machine The machine to execute the program.
 * \param self The exec_sequence_t contining the program to be executed and the nodes to be updated.
 * \return The result
 */
em_result
exec_sequence_update_value(struct machine_t * machine, exec_sequence_t * self);

// ! Assign node_t::last := node_t::value.
/* !
 * \param machine The machine. It is used for GC.
 * \param self The exec_sequence_t containing the nodes to be updated.
 * \return The result
 */
em_result
exec_sequence_update_last(struct machine_t * machine, exec_sequence_t * self);
