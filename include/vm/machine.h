/** -------------------------------------------
 * @file   machine.h
 * @brief  Emfrp REPL Machine
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/2/16
 ------------------------------------------- */

#pragma once
#include "em_result.h"
#include "collections/list_t.h"
#include "collections/dictionary_t.h"
#include "vm/node_t.h"
#include "vm/gc.h"
#include "vm/exec_sequence_t.h"
#include "vm/variable_t.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct object_t;
struct parser_node_t;
struct parser_toplevel_t;

#define MACHINE_STACK_SIZE 16

// ! Virtual Machine.
typedef struct machine_t {
  // ! The execution list with the order.
  /* !
   * Items are the pointer to the nodes' content.
   * They references machine_t::nodes;
   */
  queue_t /*<exec_sequence_t*>*/ execution_list;
  // ! The details of nodes.
  /* !
   * Items are the node(not pointer).
   */
  dictionary_t /*<node_t>*/ nodes;
  // ! The memory manager.
  memory_manager_t * memory_manager;
  // ! The stack space.
  object_t * stack;
  // ! The variable table.
  variable_table_t * variable_table;
} machine_t;

// ! Constructor of machine_t.
/* !
 * \param out The result
 * \return The status code
 */
em_result machine_new(machine_t * out);

// ! Execute the given toplevel expression.
/* !
 * \param self The machine
 * \param prog The toplevel expression
 * \param out The result if it is an expression.
 * \return The status code
 */
em_result machine_exec(machine_t * self, struct parser_toplevel_t * prog, struct object_t ** out);

// ! Add a node(with an AST program).
/* !
 * \param self The machine
 * \param n The AST of the node.
 * \return The status code
 */
em_result machine_add_node_ast(machine_t * self, exec_sequence_t ** out, struct parser_node_t * n);

// ! Add a node(a input node).
/* !
 * \param self The machine
 * \param str Name of the node
 * \param callback The callback of node. (Nullable)
 * \return The status code
 */
em_result machine_add_node_callback(machine_t * self, string_t str, exec_callback_t callback);

// ! Search value of the node.
/* !
 * \param self The machine
 * \param out The result
 * \param name Name of the node
 * \return Whether found or not
 */
bool machine_lookup_node(machine_t * self, node_t ** out, string_t * name);

// ! Push a object into the stack.
/* !
 * \param self The machine
 * \param obj The object to be pushed.
 * \return The status code.
 */
em_result machine_push(machine_t * self, object_t * obj);

// ! Pop a object from the stack.
/* ! 
 * \param self The machine
 * \param obj The object to be poped.
 * \return The status code. ( May be EM_RESULT_STACK_OVERFLOW )
 */
em_result machine_pop(machine_t * self, object_t ** obj);

// ! The stack state for exception handling.
typedef size_t stack_state_t;
#define MACHINE_STACK_STATE_DEFAULT 0

// ! Retrive the stack state.
/* !
 * \param self The machine
 * \param out The result
 * \return The status code.
 */
static inline em_result
machine_get_stack_state(machine_t * self, stack_state_t * out) {
  *out = self->stack->value.stack.length;
  return EM_RESULT_OK;
}

// ! Put a stack state.
/* !
 * \param self The machine
 * \param state The state.
 */
static inline em_result
machine_restore_stack_state(machine_t * self, stack_state_t state) {
  em_result errres = EM_RESULT_OK;
  for(int i = state; i < self->stack->value.stack.length; ++i)
    CHKERR(machine_mark_gray(self, self->stack->value.stack.data[i]));
  self->stack->value.stack.length = state;
 err: return errres;
}

#define machine_get_variable_table(self) (self)->variable_table

// ! Set a variable table.
/* !
 * \param self The machine
 * \param v A variable table to be set.
 */
static inline em_result
machine_set_variable_table(machine_t * self, variable_table_t * v) {
  em_result errres = machine_mark_gray(self, self->variable_table->this_object_ref);
  self->variable_table = v;
  return errres;
}

// ! Push a new variable table.
/* !
 * \param self The machine
 * \return The result
 */
static inline em_result
machine_new_variable_table(machine_t * self) {
  em_result errres = EM_RESULT_OK;
  variable_table_t * v = nullptr;
  CHKERR(em_malloc((void**)&v, sizeof(variable_table_t)));
  CHKERR(variable_table_new(self, v, self->variable_table));
  self->variable_table = v;
 err: return errres;
}

// ! Pop a variable table.
/* !
 * \param self The machine
 * \return The result
 */
static inline em_result
machine_pop_variable_table(machine_t * self) {
  em_result errres = machine_mark_gray(self, self->variable_table->this_object_ref);
  if(errres != EM_RESULT_OK) return errres;
  if(self->variable_table != nullptr)
    self->variable_table = self->variable_table->parent;
  return EM_RESULT_OK;
}

// ! Assign a value to the variable. 
/* !
 * \param self The machine
 * \param name The name of the variable.
 * \param value A value to be assigned.
 * \return The result
 */
static inline em_result
machine_assign_variable(machine_t * self, string_t * name, struct object_t * value) {
  return variable_table_assign(self, self->variable_table, name, value);
}

// ! Match values to the deconstructors.
/* !
 * \param self The machine
 * \param nt The deconstructor
 * \param length The length of vs
 * \param vs The pointer to the array of objects
 * \return The result
 */
em_result machine_match(machine_t * self, list_t /*<deconstructor_t>*/ * nt, struct object_t ** vs, int length);


// ! Match value to the deconstructor.
/* !
 * \param self The machine
 * \param nt The deconstructor
 * \param v The object
 * \return The result
 */
em_result machine_matches(machine_t * self, deconstructor_t * deconst, struct object_t * v);

// ! Match the values to the deconstructor.
/* !
 * \param self The machine
 * \param nt The deconstructor
 * \param v The tuple object
 * \return The result
 */
em_result machine_test_match(machine_t * self, string_t * tag, list_t /*<deconstructor_t>*/ * nt, struct object_t * v);

// ! Lookup a value of the variable.
/* !
 * \param self The machine
 * \param out The result.
 * \param name The name of the variable.
 * \return Whether found or not
 */
static inline bool
machine_lookup_variable(machine_t * self,  struct object_t ** out, string_t * name) {
  if(variable_table_lookup(self->variable_table, out, name)) return true;
  node_t * no;
  if(machine_lookup_node(self, &no, name)) {
    *out = no->value;
    return true;
  }
  return false;
}

// ! Is the node defined?
/* !
 * \param self The machine
 * \param name Name of the node
 * \return Whether found or not
 */
bool macihne_is_defined(machine_t * self, string_t * name);

// ! Indicate value of the node is changed
/* !
 * \param self The machine
 * \param name List of names of the changed node.(Nullable)
 * \return The status code
 */
em_result machine_indicate(machine_t * self, string_t * names, int count_names);

// ! Set value of the node.
/* !
 * \param self The machine
 * \param name Name of the node to be changed its value.
 * \param val the object to be set.
 */
em_result machine_set_value_of_node(machine_t * self, string_t * name, struct object_t * val);

// ! Register the output node
/* !
 * \param self The machine
 * \param name Name of the node
 * \param callback The delegate.
 * \return The status code
 */
em_result machine_add_output_node(machine_t * self, string_t name, node_event_delegate_t callback);

// ! [DEBUG] Print node definitions.
void machine_debug_print_definitions(machine_t * self);

#ifdef __cplusplus
}
#endif /* __cplusplus */
