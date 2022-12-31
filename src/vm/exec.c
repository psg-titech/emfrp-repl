/** -------------------------------------------
 * @file   exec.c
 * @brief  Emfrp REPL Interpreter Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/31
 ------------------------------------------- */

#include "vm/exec.h"

em_result
exec_ast_bin(machine_t * m, parser_expression_kind_t kind, parser_expression_t * l, parser_expression_t * r, object_t ** out) {
  object_t * lro = nullptr, * rro = nullptr;
  bool lro_is_integer, rro_is_integer;
  em_result errres;
  stack_state_t state;
  CHKERR2(err_state, machine_get_stack_state(m, &state));
  CHKERR(exec_ast(m, l, &lro));

  if(kind == EXPR_KIND_DAND && lro == &object_false) {
    *out = &object_false;
    return EM_RESULT_OK;
  } else if(kind == EXPR_KIND_DOR && lro != &object_false) {
    *out = lro;
    return EM_RESULT_OK;
  }
  CHKERR(machine_push(m, lro));
  CHKERR(exec_ast(m, r, &rro));
  lro_is_integer = object_is_integer(lro);
  rro_is_integer = object_is_integer(rro);
  CHKERR2(err_state, machine_restore_stack_state(m, state));
  if(lro_is_integer && rro_is_integer) {
    int32_t retVal = 0;
    int32_t ll = object_get_integer(lro);
    int32_t rr = object_get_integer(rro);
    switch(kind) {
    case EXPR_KIND_ADDITION: retVal = ll + rr; break;
    case EXPR_KIND_SUBTRACTION: retVal = ll - rr; break;
    case EXPR_KIND_DIVISION: retVal = ll / rr; break;
    case EXPR_KIND_MULTIPLICATION: retVal = ll * rr; break;
    case EXPR_KIND_MODULO: retVal = ll % rr; break;
    case EXPR_KIND_LEFT_SHIFT: retVal = ll << rr; break;
    case EXPR_KIND_RIGHT_SHIFT: retVal = ll >> rr; break;
    case EXPR_KIND_LESS_OR_EQUAL: retVal = ll <= rr; break;
    case EXPR_KIND_LESS_THAN: retVal = ll < rr; break;
    case EXPR_KIND_GREATER_OR_EQUAL: retVal = ll >= rr; break;
    case EXPR_KIND_GREATER_THAN: retVal = ll > rr; break;
    case EXPR_KIND_EQUAL: retVal = ll == rr; break;
    case EXPR_KIND_NOT_EQUAL: retVal = ll != rr; break;
    case EXPR_KIND_AND: retVal = ll & rr; break;
    case EXPR_KIND_OR: retVal = ll | rr; break;
    case EXPR_KIND_XOR: retVal = ll ^ rr; break;
    default: DEBUGBREAK; break;
    }
    if(kind >= EXPR_KIND_LESS_OR_EQUAL && kind <= EXPR_KIND_GREATER_THAN)
      *out = retVal ? &object_true : &object_false;
    else
      object_new_int(out, retVal);
    return EM_RESULT_OK;
  } else {
    switch(kind) {
    case EXPR_KIND_EQUAL:
      if(lro->kind != rro->kind) {
        *out = &object_false;
        goto ok;
      }
      // Add here if you added the new type.
      *out = &object_true;
      goto ok;
    case EXPR_KIND_NOT_EQUAL:
      if(lro->kind != rro->kind) {
        *out = &object_true;
        goto ok;
      }
      // Add here if you added the new type.
      *out = &object_true;
      goto ok;
    case EXPR_KIND_AND: case EXPR_KIND_DAND: // Code size than performance.
      *out = (lro != &object_false) && (rro != &object_false) ? &object_true : &object_false;
      goto ok;
    case EXPR_KIND_OR: case EXPR_KIND_DOR: // Code size than performance.
      *out = (lro != &object_false) || (rro != &object_false) ? &object_true : &object_false;
      goto ok;
    case EXPR_KIND_XOR:
      *out = (lro != &object_false) ^ (rro != &object_false) ? &object_true : &object_false;
      goto ok;
    default:
      return EM_RESULT_TYPE_MISMATCH;
    }
  ok:
    return EM_RESULT_OK;
  }

 err: machine_restore_stack_state(m, state);
 err_state: return errres;
}

em_result
exec_tuple(machine_t * m, parser_expression_tuple_list_t * li, object_t ** out) {
  em_result errres = EM_RESULT_OK;
  object_t * i0 = nullptr;
  object_t * i1 = nullptr;
  parser_expression_tuple_list_t * lli = li;
  int tuple_len = 1;
  stack_state_t state;
  CHKERR2(err_state, machine_get_stack_state(m, &state));
  while(lli->next != nullptr) {
    tuple_len++;
    lli = lli->next;
  }
  *out = nullptr;
  CHKERR(machine_alloc(m, out));
  switch(tuple_len) {
  case 1: {
    CHKERR(exec_ast(m, li->value, &i0));
    CHKERR(machine_push(m, i0));
    CHKERR(object_new_tuple1(*out, i0));
    break;
  }
  case 2: {
    CHKERR(exec_ast(m, li->value, &i0));
    CHKERR(machine_push(m, i0));
    li = li->next;
    CHKERR(exec_ast(m, li->value, &i1));
    CHKERR(machine_push(m, i1));
    CHKERR(object_new_tuple2(*out, i0, i1));
    break;
  }
  case 0:
    DEBUGBREAK;
    return EM_RESULT_INVALID_ARGUMENT;
  default: {
    int i; object_t * o;
    CHKERR(object_new_tupleN(*out, tuple_len));
    CHKERR(machine_push(m, *out));
    for(i = 0; i < tuple_len; ++i) {
      CHKERR(exec_ast(m, li->value,  &o));
      object_tuple_ith(*out, i) = o;
      li = li->next;
    }
    CHKERR(machine_pop(m, nullptr));
    break;
  }
  }
  CHKERR2(err_state, machine_restore_stack_state(m, state));
  return EM_RESULT_OK;
 err:
  machine_restore_stack_state(m, state);
  if(*out != nullptr) {
    machine_return(m, *out);
    *out = nullptr;
  }
  if(i0 != nullptr) machine_return(m, i0);
  if(i1 != nullptr) machine_return(m, i1);
 err_state: return errres;
}

em_result
exec_funccall(machine_t * m, parser_expression_t * v, object_t ** out) {
  object_t * callee = nullptr;
  object_t * args = nullptr;
  stack_state_t state;
  em_result errres = EM_RESULT_OK;
  variable_table_t * prev_vt = nullptr;
  CHKERR2(err_state, machine_get_stack_state(m, &state));
  CHKERR(exec_ast(m, v->value.funccall.callee, &callee));
  if(!object_is_pointer(callee) || (object_kind(callee) != EMFRP_OBJECT_FUNCTION))
    return EM_RESULT_TYPE_MISMATCH;
  CHKERR(machine_push(m, callee));
  if(v->value.funccall.arguments.value == nullptr) { // Arity == 0;
    if(callee->value.function.function.ast->value.function.arguments != nullptr) {
      errres = EM_RESULT_INVALID_ARGUMENT;
      goto err;
    }
  } else {
    CHKERR(exec_tuple(m, &(v->value.funccall.arguments), &args));
    CHKERR(machine_push(m, args));
  }
  prev_vt = machine_get_variable_table(m);
  CHKERR(machine_push(m, prev_vt->this_object_ref));
  if(!object_is_pointer(callee->value.function.closure) || callee->value.function.closure == nullptr) {
    CHKERR(machine_set_variable_table(m, nullptr));
  } else if(object_kind(callee->value.function.closure) == EMFRP_OBJECT_VARIABLE_TABLE) {
    CHKERR(machine_set_variable_table(m, callee->value.function.closure->value.variable_table.ptr));
  } else {
    errres = EM_RESULT_INVALID_ARGUMENT;
    goto err;
  }
  CHKERR2(err2, machine_new_variable_table(m));
  switch(callee->value.function.kind) {
  case EMFRP_PROGRAM_KIND_AST:
    CHKERR2(err2, machine_assign_variable_tuple(m, callee->value.function.function.ast->value.function.arguments, args));
    CHKERR2(err2, exec_ast(m, callee->value.function.function.ast->value.function.body, out)); break;
  default: DEBUGBREAK; break;
  }
  CHKERR2(err2, machine_pop_variable_table(m));
  machine_set_variable_table(m, prev_vt);
  machine_restore_stack_state(m, state);
  return EM_RESULT_OK;
 err2: machine_set_variable_table(m, prev_vt);
 err:  machine_restore_stack_state(m, state);
 err_state: return errres;
}

em_result
exec_func(machine_t * m, parser_expression_t * v, object_t ** out) {
  em_result errres = EM_RESULT_OK;
  CHKERR(machine_alloc(m, out));
  return object_new_function_ast(*out, machine_get_variable_table(m)->this_object_ref, v);
 err: return errres;
}

em_result
exec_ast(machine_t * m, parser_expression_t * v, object_t ** out) {
  em_result errres;
  node_t * id;
  stack_state_t state;
  if (EXPR_KIND_IS_INTEGER(v)) {
    CHKERR2(err_state, object_new_int(out, (int32_t)((size_t)v >> 1)));
  } else if (EXPR_KIND_IS_BOOLEAN(v))
    *out = EXPR_IS_TRUE(v) ? &object_true : &object_false;
  else if (EXPR_KIND_IS_BIN_OP(v))
    return exec_ast_bin(m, v->kind, v->value.binary.lhs, v->value.binary.rhs, out);
  else {
    switch(v->kind) {
    case EXPR_KIND_IF: {
      object_t * cond_result = nullptr;
      bool cond_v;
      CHKERR2(err_state, machine_get_stack_state(m, &state));
      CHKERR(exec_ast(m, v->value.ifthenelse.cond, &cond_result));
      CHKERR(machine_push(m, cond_result));
      cond_v = cond_result == &object_true;
      machine_restore_stack_state(m, state);
      CHKERR(exec_ast(m, cond_v ? v->value.ifthenelse.then : v->value.ifthenelse.otherwise, out));
      break;
    }
    case EXPR_KIND_IDENTIFIER:
      if(!machine_lookup_variable(m, out, &(v->value.identifier)))        
        return EM_RESULT_MISSING_IDENTIFIER;
      break;
    case EXPR_KIND_LAST_IDENTIFIER:
      if(!machine_lookup_node(m, &id, &(v->value.identifier)))
        return EM_RESULT_MISSING_IDENTIFIER;
      *out = id->last;
      break;
    case EXPR_KIND_TUPLE:
      return exec_tuple(m, &(v->value.tuple), out);
    case EXPR_KIND_FUNCCALL: return exec_funccall(m, v, out);
    case EXPR_KIND_FUNCTION: return exec_func(m, v, out);
    default:
      return EM_RESULT_INVALID_ARGUMENT;
    }
  }
  return EM_RESULT_OK;
 err: machine_restore_stack_state(m, state);
 err_state: return errres;
}

em_result
get_dependencies_ast(parser_expression_t * v, list_t /*<string_t *>*/ ** out) {
  em_result errres;
  if(EXPR_KIND_IS_INTEGER(v) || EXPR_KIND_IS_BOOLEAN(v)) return EM_RESULT_OK;
  if(v->kind == EXPR_KIND_IDENTIFIER) {
    string_t * s = &(v->value.identifier);
    CHKERR(list_add2(out, string_t *, &s));
  } else if(EXPR_KIND_IS_BIN_OP(v)){
    CHKERR(get_dependencies_ast(v->value.binary.lhs, out));
    CHKERR(get_dependencies_ast(v->value.binary.rhs, out));
  } else switch(v->kind) {
    case EXPR_KIND_TUPLE:
      for(parser_expression_tuple_list_t * li = &(v->value.tuple);
          li != nullptr; li = li->next)
        CHKERR(get_dependencies_ast(li->value, out));
      break;
    case EXPR_KIND_IF:
      CHKERR(get_dependencies_ast(v->value.ifthenelse.cond, out));
      CHKERR(get_dependencies_ast(v->value.ifthenelse.then, out));
      CHKERR(get_dependencies_ast(v->value.ifthenelse.otherwise, out));
      break;
    case EXPR_KIND_FUNCCALL:
      CHKERR(get_dependencies_ast(v->value.funccall.callee, out));
      if(v->value.funccall.arguments.value != nullptr) {
        for(parser_expression_tuple_list_t * li = &(v->value.funccall.arguments);
            li != nullptr; li = li->next)
          CHKERR(get_dependencies_ast(li->value, out));
      }
      break;
    default: break;
    }
  return EM_RESULT_OK;
 err:
  return errres;
}

bool
check_depends_on_ast(parser_expression_t * v, string_t * str) {
  if (EXPR_KIND_IS_INTEGER(v) || EXPR_KIND_IS_BOOLEAN(v)) return false;
  if(v->kind == EXPR_KIND_IDENTIFIER)
    return string_compare(&(v->value.identifier), str);
  else if(EXPR_KIND_IS_BIN_OP(v))
    return check_depends_on_ast(v->value.binary.lhs, str) || check_depends_on_ast(v->value.binary.rhs, str);
  else switch(v->kind) {
    case EXPR_KIND_TUPLE:
      for(parser_expression_tuple_list_t * li = &(v->value.tuple);
          li != nullptr; li = li->next)
        if(check_depends_on_ast(li->value, str)) return true;
      return false;	 
    case EXPR_KIND_IF:
      return check_depends_on_ast(v->value.ifthenelse.cond, str)
        || check_depends_on_ast(v->value.ifthenelse.then, str)
        || check_depends_on_ast(v->value.ifthenelse.otherwise, str);
    case EXPR_KIND_FUNCCALL:
      if(check_depends_on_ast(v->value.funccall.callee, str)) return true;
      if(v->value.funccall.arguments.value != nullptr) {
        for(parser_expression_tuple_list_t * li = &(v->value.funccall.arguments);
            li != nullptr; li = li->next)
          if(check_depends_on_ast(li->value, str)) return true;
      }
      return false;	 
  }
  return false;
}
