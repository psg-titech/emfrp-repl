/** -------------------------------------------
 * @file   exec.c
 * @brief  Emfrp REPL Interpreter Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/14
 ------------------------------------------- */

#include "vm/exec.h"

em_result
exec_ast_bin(machine_t * m, parser_expression_kind_t kind, parser_expression_t * l, parser_expression_t * r, object_t ** out) {
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

em_result
exec_ast(machine_t * m, parser_expression_t * v, object_t ** out) {
  em_result errres;
  if(EXPR_KIND_IS_BIN_OP(v)) {
    CHKERR(exec_ast_bin(m, v->kind, v->value.binary.lhs, v->value.binary.rhs, out));
  } else {
    switch(v->kind) {
    case EXPR_KIND_INTEGER:
      object_new_int(out, v->value.integer);
      break;
    case EXPR_KIND_IDENTIFIER:
      if(m->executing_node_name != nullptr && string_compare(&(v->value.identifier), m->executing_node_name))
	return EM_RESULT_CYCLIC_REFERENCE;
      if(!machine_search_node(m, out, &(v->value.identifier)))
	return EM_RESULT_MISSING_IDENTIFIER;
      break;
    case EXPR_KIND_LAST_IDENTIFIER:
      if(m->executing_node_name != nullptr && !string_compare(&(v->value.identifier), m->executing_node_name))
	return EM_RESULT_MISSING_IDENTIFIER;
      if(!machine_search_node(m, out, &(v->value.identifier)))
	return EM_RESULT_MISSING_IDENTIFIER;
      break;
    default:
      return EM_RESULT_INVALID_ARGUMENT;
    }
  }
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
get_dependencies_ast(parser_expression_t * v, list_t /*<string_t *>*/ ** out) {
  em_result errres;
  if(v->kind == EXPR_KIND_IDENTIFIER) {
    string_t * s = &(v->value.identifier);
    CHKERR(list_add2(out, string_t *, &s));
  } else if(EXPR_KIND_IS_BIN_OP(v)){
    CHKERR(get_dependencies_ast(v->value.binary.lhs, out));
    CHKERR(get_dependencies_ast(v->value.binary.rhs, out));
  }
  return EM_RESULT_OK;
 err:
  return errres;
}

bool
check_depends_on_ast(parser_expression_t * v, string_t * str) {
  if(v->kind == EXPR_KIND_IDENTIFIER)
    return string_compare(&(v->value.identifier), str);
  else if(EXPR_KIND_IS_BIN_OP(v))
    return check_depends_on_ast(v->value.binary.lhs, str) || check_depends_on_ast(v->value.binary.rhs, str);
  return false;
}
