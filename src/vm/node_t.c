/** -------------------------------------------
 * @file   node_t.c
 * @brief  Emfrp Node Implementation(IR)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/11
 ------------------------------------------- */

#include "vm/node_t.h"

em_result
node_new_ast(node_t * out, string_t name, parser_expression_t * ast) {
  out->name = name;
  out->program_kind = NODE_PROGRAM_KIND_AST;
  out->program.ast = ast;
  out->value = nullptr;
  out->action = nullptr;
  return EM_RESULT_OK;
}

em_result
node_new_callback(node_t * out, string_t name, node_callback_t callback) {
  out->name = name;
  out->program_kind = NODE_PROGRAM_KIND_CALLBACK;
  out->program.callback = callback;
  out->value = nullptr;
  out->action = nullptr;
  return EM_RESULT_OK;
}

void
node_deep_free(node_t * v) {
  string_free(&(v->name));
  switch(v->program_kind) {
  case NODE_PROGRAM_KIND_AST:
    parser_expression_free(v->program.ast);
    break;
  default: break;
  }
  object_free(v->value);
}
