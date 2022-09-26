/** -------------------------------------------
 * @file   exec.h
 * @brief  Emfrp REPL Interpreter
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/26
 ------------------------------------------- */

#pragma once

#include "vm/object_t.h"
#include "vm/machine.h"
#include "ast.h"

// ! Execute AST.
/* !
 * \param m The machine.
 * \param v The expression to be executed.
 * \param out The result.
 * \return The status code.
 */
em_result
exec_ast(machine_t * m, parser_expression_t * v, object_t ** out);
