/** -------------------------------------------
 * @file   machine.h
 * @brief  Emfrp REPL Machine
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/2
 ------------------------------------------- */

#pragma once
#include "em_result.h"
#include "collections/list_t.h"
#include "collections/dictionary_t.h"
#include "ast.h"
#include "vm/node_t.h"
#include "vm/object_t.h"
#include "vm/gc.h"
#include "vm/exec_sequence_t.h"

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
} machine_t;

// ! Constructor of machine_t.
/* !
 * \param out The result
 * \return The status code
 */
em_result machine_new(machine_t * out);

// ! Add a node(with an AST program).
/* !
 * \param self The machine
 * \param str Name of the node
 * \param prog Program of the node
 * \param initialization The initialization value, not freed, please free by the caller.
 * \return The status code
 */
em_result machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog, parser_expression_t * initialization);

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
em_result machine_set_value_of_node(machine_t * self, string_t * name, object_t * val);


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
