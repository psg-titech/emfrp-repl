/** -------------------------------------------
 * @file   exec.c
 * @brief  Emfrp REPL Interpreter Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/2/4
 ------------------------------------------- */

#include "vm/exec.h"

bool
exec_equal(object_t * l, object_t * r) {
  bool b = true;
  if(!object_is_pointer(l) && !object_is_pointer(r))
    return l == r;
  if(object_kind(l) != object_kind(r)) return false;
  switch(object_kind(l)) {
  case EMFRP_OBJECT_TUPLE1:
    return exec_equal(l->value.tuple1.tag, r->value.tuple1.tag)
      && exec_equal(l->value.tuple1.i0, r->value.tuple1.i0);
  case EMFRP_OBJECT_TUPLE2:
    return exec_equal(l->value.tuple2.tag, r->value.tuple2.tag)
      && exec_equal(l->value.tuple2.i0, r->value.tuple2.i0)
      && exec_equal(l->value.tuple2.i1, r->value.tuple2.i1);
  case EMFRP_OBJECT_TUPLEN:
    if(!exec_equal(l->value.tupleN.tag, r->value.tupleN.tag)
       || l->value.tupleN.length != r->value.tupleN.length) return false;
    for(int i = 0; i < l->value.tupleN.length; ++i)
      b &= exec_equal(l->value.tupleN.data[i], r->value.tupleN.data[i]);
    return b;
  case EMFRP_OBJECT_SYMBOL:
  case EMFRP_OBJECT_STRING:
    return string_compare(&(l->value.symbol.value), &(r->value.symbol.value));
  case EMFRP_OBJECT_FUNCTION:
  case EMFRP_OBJECT_VARIABLE_TABLE:
    return false;
  default: DEBUGBREAK; return false;
  }
}

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
    int retVal = 0;
    int ll = object_get_integer(lro);
    int rr = object_get_integer(rro);
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
    if(kind >= EXPR_KIND_LESS_OR_EQUAL && kind <= EXPR_KIND_NOT_EQUAL)
      *out = retVal ? &object_true : &object_false;
    else
      object_new_int(out, retVal);
    return EM_RESULT_OK;
  } else {
    switch(kind) {
    case EXPR_KIND_EQUAL:
      *out = exec_equal(lro, rro) ? &object_true : &object_false;
      goto ok;
    case EXPR_KIND_NOT_EQUAL:
      *out = exec_equal(lro, rro) ? &object_false : &object_true;
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
exec_construct_record(object_t * tag, size_t arity, object_t * args, object_t ** out) {
  if(arity == 0 && args == nullptr) {
    *out = tag;
    return EM_RESULT_OK;
  } if(arity == 1 && object_kind(args) == EMFRP_OBJECT_TUPLE1) {
    args->value.tuple1.tag = tag;
    *out = args;
    return EM_RESULT_OK;
  } else if(arity == 2 && object_kind(args) == EMFRP_OBJECT_TUPLE2) {
    args->value.tuple2.tag = tag;
    *out = args;
    return EM_RESULT_OK;
  } else if(arity >= 3 && object_kind(args) == EMFRP_OBJECT_TUPLEN
	    && args->value.tupleN.length) {
    args->value.tupleN.tag = tag;
    *out = args;
    return EM_RESULT_OK;
  }
  return EM_RESULT_INVALID_ARGUMENT;
}

em_result
exec_access_record(object_t * tag, size_t index, object_t * args, object_t ** out) {
  if(object_kind(args) != EMFRP_OBJECT_TUPLE1)
    return EM_RESULT_INVALID_ARGUMENT;
  object_t * t = args->value.tuple1.i0;
  if(!object_is_pointer(t) || t == nullptr) return EM_RESULT_TYPE_MISMATCH;
  switch(object_kind(t))  {
  case EMFRP_OBJECT_TUPLE1:
    if(!exec_equal(t->value.tuple1.tag, tag))
      return EM_RESULT_TYPE_MISMATCH;
    if(index >= 1) return EM_RESULT_TYPE_MISMATCH;
    *out = t->value.tuple1.i0;
    return EM_RESULT_OK;
  case EMFRP_OBJECT_TUPLE2:
    if(!exec_equal(t->value.tuple2.tag, tag))
      return EM_RESULT_TYPE_MISMATCH;
    if(index >= 2) return EM_RESULT_TYPE_MISMATCH;
    *out = index == 0 ? t->value.tuple2.i0 : t->value.tuple2.i1;
    return EM_RESULT_OK;
  case EMFRP_OBJECT_TUPLEN:
    if(!exec_equal(t->value.tupleN.tag, tag))
      return EM_RESULT_TYPE_MISMATCH;
    if(index >= t->value.tupleN.length) return EM_RESULT_TYPE_MISMATCH;
    *out = object_tuple_ith(t, index);
    return EM_RESULT_OK;
  default:
    return EM_RESULT_TYPE_MISMATCH;
  }
}

em_result
exec_funccall(machine_t * m, parser_expression_t * v, object_t ** out) {
  object_t * callee = nullptr;
  object_t * args = nullptr;
  stack_state_t state = MACHINE_STACK_STATE_DEFAULT;
  em_result errres = EM_RESULT_OK;
  variable_table_t * prev_vt = nullptr;
  CHKERR2(err_state, machine_get_stack_state(m, &state));
  CHKERR(exec_ast(m, v->value.funccall.callee, &callee));
  if(!object_is_pointer(callee) || callee == nullptr || (object_kind(callee) != EMFRP_OBJECT_FUNCTION))
    return EM_RESULT_TYPE_MISMATCH;
  CHKERR(machine_push(m, callee));
  if(v->value.funccall.arguments.value != nullptr) {
    CHKERR(exec_tuple(m, &(v->value.funccall.arguments), &args));
    CHKERR(machine_push(m, args));
  }
  switch(callee->value.function.kind) {
  case EMFRP_PROGRAM_KIND_AST:
    if(v->value.funccall.arguments.value == nullptr) { // Arity == 0;
      if(callee->value.function.function.ast.program->value.function.arguments != nullptr) {
	errres = EM_RESULT_INVALID_ARGUMENT;
	goto err;
      }
    }
    prev_vt = machine_get_variable_table(m);
    CHKERR(machine_push(m, prev_vt->this_object_ref));
    if(!object_is_pointer(callee->value.function.function.ast.closure)
       || callee->value.function.function.ast.closure == nullptr) {
      CHKERR(machine_set_variable_table(m, nullptr));
    } else if(object_kind(callee->value.function.function.ast.closure) == EMFRP_OBJECT_VARIABLE_TABLE) {
      CHKERR(machine_set_variable_table(m, callee->value.function.function.ast.closure->value.variable_table.ptr));
    } else {
      errres = EM_RESULT_INVALID_ARGUMENT;
      goto err;
    }
    CHKERR2(err2, machine_push(m, prev_vt->this_object_ref));
    CHKERR2(err2, machine_new_variable_table(m));
    CHKERR2(err2, machine_assign_variable_tuple(m, nullptr, callee->value.function.function.ast.program->value.function.arguments, args));
    CHKERR2(err2, exec_ast(m, callee->value.function.function.ast.program->value.function.body, out));
    machine_set_variable_table(m, prev_vt);
    break;
  case EMFRP_PROGRAM_KIND_NOTHING: break;
  case EMFRP_PROGRAM_KIND_CALLBACK: CHKERR(callee->value.function.function.callback(out, args)); break;
  case EMFRP_PROGRAM_KIND_RECORD_CONSTRUCT:
    CHKERR(exec_construct_record(callee->value.function.function.construct.tag,
				 callee->value.function.function.construct.arity,
				 args, out)); break;
  case EMFRP_PROGRAM_KIND_RECORD_ACCESS:
    CHKERR(exec_access_record(callee->value.function.function.access.tag,
			      callee->value.function.function.access.index,
			      args, out)); break;
  default: DEBUGBREAK; break;
  }
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
exec_caseof(machine_t * m, object_t * v, parser_branch_list_t * bl, object_t ** out) {
  while(bl != nullptr) {
    bool isOk = false;
    switch(bl->deconstruct->kind) {
    case DECONSTRUCTOR_IDENTIFIER:
    case DECONSTRUCTOR_ANY: isOk = true; break;
    case DECONSTRUCTOR_TUPLE:
      isOk = machine_match_test(m, bl->deconstruct->value.tuple.tag, bl->deconstruct->value.tuple.data, v) == EM_RESULT_OK;
      break;
    case DECONSTRUCTOR_INTEGER:
      isOk = (object_is_integer(v) && bl->deconstruct->value.integer == object_get_integer(v));
      break;
#if EMFRP_ENABLE_FLOATING
    case DECONSTRUCTOR_FLOAT: break;
#endif
    default:DEBUGBREAK; break;
    }
    if(isOk) {
      em_result errres = EM_RESULT_OK;
      bool isVariableTableCreated  = false;
      if((machine_get_variable_table(m)) == nullptr
	 || (machine_get_variable_table(m))->parent == nullptr) {
	isVariableTableCreated = true;
	CHKERR2(err2, machine_new_variable_table(m));
      }
      if(bl->deconstruct->kind == DECONSTRUCTOR_TUPLE) {
	CHKERR(machine_assign_variable_tuple(m, bl->deconstruct->value.tuple.tag, bl->deconstruct->value.tuple.data, v));
      } else if(bl->deconstruct->kind == DECONSTRUCTOR_IDENTIFIER)
	CHKERR(machine_assign_variable(m, bl->deconstruct->value.identifier, v));
      CHKERR(exec_ast(m, bl->body, out));
    err:
      if(isVariableTableCreated)
	machine_pop_variable_table(m);
    err2:
      return errres;
    }
    bl = bl->next;
  }
  if(bl == nullptr) *out = nullptr;
  return EM_RESULT_OK;
}

em_result
exec_begin(machine_t * m, parser_branch_list_t * bl, object_t ** out) {
  em_result errres = EM_RESULT_OK;
  object_t * o = nullptr;
  bool isVariableTableCreated = false;
  for(;bl != nullptr; bl = bl->next) {
    CHKERR(exec_ast(m, bl->body, &o));
    if(bl->deconstruct != nullptr) {
      CHKERR(machine_new_variable_table(m));
      goto assign;
    }
  }
  goto err;
  for(; bl != nullptr; bl = bl->next) {
    CHKERR(exec_ast(m, bl->body, &o));
    if(bl->deconstruct != nullptr) {
    assign:
      switch(bl->deconstruct->kind) {
      case DECONSTRUCTOR_IDENTIFIER:
	CHKERR(machine_assign_variable(m, bl->deconstruct->value.identifier, o));
      case DECONSTRUCTOR_ANY: break;
      case DECONSTRUCTOR_TUPLE:
	CHKERR(machine_assign_variable_tuple(m, bl->deconstruct->value.tuple.tag, bl->deconstruct->value.tuple.data, o));
	break;
      default:DEBUGBREAK; break;
      }
    }
  }
  machine_pop_variable_table(m);
 err: *out = o; return errres;
}

em_result
exec_ast(machine_t * m, parser_expression_t * v, object_t ** out) {
  if (EXPR_KIND_IS_INTEGER(v)) {
    return object_new_int(out, (int)((size_t)v >> 2));
  } else if (EXPR_KIND_IS_BOOLEAN(v))
    *out = EXPR_IS_TRUE(v) ? &object_true : &object_false;
  else if (EXPR_KIND_IS_BIN_OP(v))
    return exec_ast_bin(m, v->kind, v->value.binary.lhs, v->value.binary.rhs, out);
  else {
    switch(v->kind) {
    case EXPR_KIND_IDENTIFIER:
      return machine_lookup_variable(m, out, &(v->value.identifier)) ?
		      EM_RESULT_OK : EM_RESULT_MISSING_IDENTIFIER;
    case EXPR_KIND_IF: {
      em_result errres = EM_RESULT_OK;
      object_t * cond_result = nullptr;
      stack_state_t state = MACHINE_STACK_STATE_DEFAULT;
      CHKERR2(err_if2, machine_get_stack_state(m, &state));
      CHKERR2(err_if2, exec_ast(m, v->value.ifthenelse.cond, &cond_result));
      CHKERR2(err_if, machine_push(m, cond_result));
      CHKERR2(err_if, exec_ast(m, cond_result == &object_true ? v->value.ifthenelse.then : v->value.ifthenelse.otherwise, out));
    err_if:  machine_restore_stack_state(m, state);
    err_if2: return errres;
    }
    case EXPR_KIND_CASE: {
      em_result errres = EM_RESULT_OK;
      object_t * v_result = nullptr;
      stack_state_t state = MACHINE_STACK_STATE_DEFAULT;
      CHKERR2(err_case2, machine_get_stack_state(m, &state));
      CHKERR2(err_case2, exec_ast(m, v->value.caseof.of, &v_result));
      CHKERR2(err_case, machine_push(m, v_result));
      CHKERR2(err_case, exec_caseof(m, v_result, v->value.caseof.branches, out));
    err_case: machine_restore_stack_state(m, state);
    err_case2: return errres;
    }
    case EXPR_KIND_BEGIN: return exec_begin(m, v->value.begin.branches, out); break;
    case EXPR_KIND_LAST_IDENTIFIER: {
      node_t * id;
      if(!machine_lookup_node(m, &id, &(v->value.identifier)))
        return EM_RESULT_MISSING_IDENTIFIER;
      *out = id->last;
      break;
    }
    case EXPR_KIND_TUPLE:    return exec_tuple(m, &(v->value.tuple), out);
    case EXPR_KIND_FUNCCALL: return exec_funccall(m, v, out);
    case EXPR_KIND_FUNCTION: return exec_func(m, v, out);
    default:                 return EM_RESULT_INVALID_ARGUMENT;
    }
  }
  return EM_RESULT_OK;
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
     default: break;
  }
  return false;
}
