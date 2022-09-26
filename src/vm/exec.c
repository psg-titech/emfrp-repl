/** -------------------------------------------
 * @file   exec.c
 * @brief  Emfrp REPL Interpreter Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/26
 ------------------------------------------- */

#include "vm/exec.h"

em_result exec_ast_bin(machine_t * m, parser_expression_kind_t kind, parser_expression_t * l, parser_expression_t * r, object_t ** out) {
  object_t * lro = nullptr, * rro = nullptr;
  em_result errres;
  CHKERR(exec_ast(m, l, &lro));
  CHKERR(exec_ast(m, r, &rro));

  if(object_is_integer(lro) && object_is_integer(rro)) {
    int32_t retVal = 0;
    int32_t ll = object_get_integer(lro);
    int32_t rr = object_get_integer(rro);
    switch(kind) {
    case EXPR_KIND_ADDITION: retVal = ll + rr; break;
    case EXPR_KIND_SUBTRACTION: retVal = ll - rr; break;
    case EXPR_KIND_DIVISION: retVal = ll / rr; break;
    case EXPR_KIND_MULTIPLICATION: retVal = ll * rr; break;
    }
    object_new_int(out, retVal);
    return EM_RESULT_OK;
  }else {
    object_free(lro);
    object_free(rro);
    return EM_RESULT_TYPE_MISMATCH;
  }

 err:
  return errres;
}

em_result exec_ast(machine_t * m, parser_expression_t * v, object_t ** out) {
  em_result errres;
  if(v->kind & 1 == 1) {
    CHKERR(exec_ast_bin(m, v->kind, v->value.binary.lhs, v->value.binary.rhs, out));
  } else {
    switch(v->kind) {
    case EXPR_KIND_INTEGER:
      object_new_int(out, v->value.integer);
      break;
    default:
      return EM_RESULT_INVALID_ARGUMENT;
    }
  }
  return EM_RESULT_OK;
 err:
  return errres;
}
