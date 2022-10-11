/** -------------------------------------------
 * @file   machine.h
 * @brief  Emfrp REPL Machine
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/11
 ------------------------------------------- */

#pragma once
#include "em_result.h"
#include "collections/list_t.h"
#include "collections/dictionary_t.h"
#include "ast.h"
#include "vm/node_t.h"
#include "vm/object_t.h"

typedef struct machine_t {
  queue_t /*<node_t*>*/ execution_list;
  dictionary_t /*<node_t>*/ nodes;
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
 * \return The status code
 */
em_result machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog);

// ! Add a node(a input node).
/* !
 * \param self The machine
 * \param str Name of the node
 * \param callback The callback of node. (Nullable)
 * \return The status code
 */
em_result machine_add_node_callback(machine_t * self, string_t str, node_callback_t callback);

// ! Search value of the node.
/* !
 * \param self The machine
 * \param out The result
 * \param name Name of the node
 * \return Whether found or not
 */
bool machine_search_node(machine_t * self, object_t ** out, string_t * name);

// ! Is the node defined?
/* !
 * \param self The machine
 * \param name Name of the node
 * \return Whether found or not
 */
bool macihne_is_defined(machine_t * self, string_t * name);

// ! Indicate Changed
/* !
 * \param self The machine
 * \param name List of names of the changed node.(Nullable)
 * \return The status code
 */
em_result machine_indicate(machine_t * self, string_t * names);

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
