/** -------------------------------------------
 * @file   node_t.c
 * @brief  Emfrp Node Implementation(IR)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/27
 ------------------------------------------- */

#include "vm/node_t.h"

em_result
node_new_ast(node_t * out, string_t name, parser_expression_t * ast) {
  out->name = name;
  out->program_kind = NODE_PROGRAM_KIND_AST;
  out->program.ast = ast;
  return EM_RESULT_OK;
}
