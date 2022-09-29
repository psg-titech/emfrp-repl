/** -------------------------------------------
 * @file   exec.h
 * @brief  Emfrp REPL Interpreter
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/29
 ------------------------------------------- */

#pragma once

#include "vm/object_t.h"
#include "vm/machine.h"
#include "ast.h"
#include "collections/list_t.h"

// ! Execute AST.
/* !
 * \param m The machine.
 * \param v The expression to be executed.
 * \param out The result.
 * \return The status code.
 */
em_result exec_ast(machine_t * m, parser_expression_t * v, object_t ** out);

// ! Get dependencies of give AST.
/* !
 * \param v The expression
 * \param out The result
 * \return The status code
 */
em_result get_dependencies_ast(parser_expression_t * v, list_t /*<string_t>*/ ** out);

// ! Check the given identifier is referenced from v.
/* !
 * \param v The expression
 * \param str The string which refers
 * \return Whether v references str.
 */
bool check_depends_on_ast(parser_expression_t * v, string_t * str);
