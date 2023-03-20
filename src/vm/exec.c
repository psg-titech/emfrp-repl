/** -------------------------------------------
 * @file   exec.c
 * @brief  Emfrp REPL Interpreter Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/20
 ------------------------------------------- */

#include "vm/exec.h"

typedef struct exec_result_t {
  object_t * value;
  enum { EXEC_RESULT_OBJECT, EXEC_RESULT_EXECUTE_FUNCTION } kind;
  stack_state_t stack_state;
  stack_state_t arglen;
} exec_result_t;

typedef em_result (*executor)(machine_t * m, parser_expression_t * v, exec_result_t * out);

em_result exec_ast_mono(machine_t * m, parser_expression_t * v, exec_result_t * out);

bool
exec_equal(object_t * l, object_t * r) {
  bool b = true;
  if(l == r)
    return true;
  if(!object_is_pointer(l) || !object_is_pointer(r)) return false;
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
  default: return false;
  }
}

#define BIN_OP_NUM_NUM_NUM_FUNC(func_name, expression) em_result	\
  func_name(machine_t * m, parser_expression_t * v, exec_result_t * out) { \
    object_t * lro = nullptr, * rro = nullptr;				\
    em_result errres = EM_RESULT_OK;					\
    int ll, rr;								\
    CHKERR(exec_ast(m, v->value.binary.lhs, &lro));			\
    TEST_AND_ERROR(!object_is_integer(lro), EM_RESULT_TYPE_MISMATCH);	\
    CHKERR(exec_ast(m, v->value.binary.rhs, &rro));			\
    TEST_AND_ERROR(!object_is_integer(rro), EM_RESULT_TYPE_MISMATCH);	\
    ll = object_get_integer(lro);					\
    rr = object_get_integer(rro);					\
    object_new_int(&(out->value), expression);			\
  err: return errres;							\
  }

#define BIN_OP_NUM_NUM_BOOL_FUNC(func_name, expression) em_result	\
  func_name(machine_t * m, parser_expression_t * v, exec_result_t * out) { \
    object_t * lro = nullptr, * rro = nullptr;				\
    em_result errres = EM_RESULT_OK;					\
    int ll, rr;								\
    CHKERR(exec_ast(m, v->value.binary.lhs, &lro));			\
    TEST_AND_ERROR(!object_is_integer(lro), EM_RESULT_TYPE_MISMATCH);	\
    CHKERR(exec_ast(m, v->value.binary.rhs, &rro));			\
    TEST_AND_ERROR(!object_is_integer(rro), EM_RESULT_TYPE_MISMATCH);	\
    ll = object_get_integer(lro);					\
    rr = object_get_integer(rro);					\
    out->value = (expression) ? &object_true : &object_false;	\
  err: return errres;							\
  }
  
BIN_OP_NUM_NUM_NUM_FUNC(exec_ast_addition, ll + rr);
BIN_OP_NUM_NUM_NUM_FUNC(exec_ast_subtract, ll - rr);
BIN_OP_NUM_NUM_NUM_FUNC(exec_ast_division, ll / rr);
BIN_OP_NUM_NUM_NUM_FUNC(exec_ast_multiplication, ll * rr);
BIN_OP_NUM_NUM_NUM_FUNC(exec_ast_modulo, ll % rr);
BIN_OP_NUM_NUM_NUM_FUNC(exec_ast_right_shift, ll >> rr);
BIN_OP_NUM_NUM_NUM_FUNC(exec_ast_left_shift, ll << rr);
BIN_OP_NUM_NUM_BOOL_FUNC(exec_ast_less_or_equal, ll <= rr);
BIN_OP_NUM_NUM_BOOL_FUNC(exec_ast_less_than, ll < rr);
BIN_OP_NUM_NUM_BOOL_FUNC(exec_ast_greater_or_equal, ll >= rr);
BIN_OP_NUM_NUM_BOOL_FUNC(exec_ast_greater_than, ll > rr);

em_result
exec_ast_equal(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  object_t * lro = nullptr, * rro = nullptr;
  em_result errres = EM_RESULT_OK;
  CHKERR(exec_ast(m, v->value.binary.lhs, &lro));
  CHKERR(machine_push(m, lro));
  CHKERR(exec_ast(m, v->value.binary.rhs, &rro));
  out->value = exec_equal(lro, rro) ? &object_true : &object_false;
 err: return errres;  
}

em_result
exec_ast_not_equal(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  object_t * lro = nullptr, * rro = nullptr;
  em_result errres = EM_RESULT_OK;
  CHKERR(exec_ast(m, v->value.binary.lhs, &lro));
  CHKERR(machine_push(m, lro));
  CHKERR(exec_ast(m, v->value.binary.rhs, &rro));
  out->value = exec_equal(lro, rro) ? &object_false : &object_true;
 err: return errres;  
}

#define BIN_OP_BOOL_BOOL_BOOL_FUNC(func_name, expression) em_result	\
  func_name(machine_t * m, parser_expression_t * v, exec_result_t * out) { \
    object_t * lro = nullptr, * rro = nullptr;				\
    em_result errres = EM_RESULT_OK;					\
    CHKERR(exec_ast(m, v->value.binary.lhs, &lro));			\
    CHKERR(machine_push(m, lro));					\
    CHKERR(exec_ast(m, v->value.binary.rhs, &rro));			\
    out->value = (expression) ? &object_true : &object_false;		\
  err: return errres;							\
  }

#define BIN_OP_BOOL_BOOL_BOOL_FUNC2(func_name, expression_short, expression) em_result \
  func_name(machine_t * m, parser_expression_t * v, exec_result_t * out) { \
    object_t * lro = nullptr, * rro = nullptr;				\
    em_result errres = EM_RESULT_OK;					\
    CHKERR(exec_ast(m, v->value.binary.lhs, &lro));			\
    if(expression_short) {						\
      out->value = lro;							\
      return EM_RESULT_OK;						\
    }									\
    CHKERR(machine_push(m, lro));					\
    CHKERR(exec_ast(m, v->value.binary.rhs, &rro));			\
    out->value = (expression) ? &object_true : &object_false;		\
  err: return errres;							\
  }

BIN_OP_BOOL_BOOL_BOOL_FUNC(exec_ast_and, (lro != &object_false) && (rro != &object_false));
BIN_OP_BOOL_BOOL_BOOL_FUNC(exec_ast_or, (lro != &object_false) || (rro != &object_false));
BIN_OP_BOOL_BOOL_BOOL_FUNC(exec_ast_xor, (lro != &object_false) ^ (rro != &object_false));
BIN_OP_BOOL_BOOL_BOOL_FUNC2(exec_ast_dand, lro == &object_false, rro != &object_false);
BIN_OP_BOOL_BOOL_BOOL_FUNC2(exec_ast_dor, lro != &object_false, rro != &object_false);

em_result
exec_ast_tuple_list_t(machine_t * m, parser_expression_tuple_list_t * li, int * length) {
  em_result errres = EM_RESULT_OK;
  int len = 0;
  *length = 0;
  for(;li != nullptr; li = LIST_NEXT(li)) {
    object_t * cur = nullptr;
    len++;
    CHKERR(exec_ast(m, li->value, &cur));
    CHKERR(machine_push(m, cur));
  }
  *length = len;
 err: return errres;
}

em_result
exec_ast_construct_tuple(machine_t * m, object_t * tag, int len, object_t ** buf, object_t ** out) {
  if(len == 0) {
    *out = tag;
    return EM_RESULT_OK;
  } else {
    em_result errres = EM_RESULT_OK;
    object_t * ret = nullptr;
    CHKERR(machine_alloc(m, &ret));
    if(len == 1) {
      CHKERR(object_new_tuple1(ret, buf[0]));
      ret->value.tuple1.tag = tag;
    } else if(len == 2) {
      CHKERR(object_new_tuple2(ret, buf[0], buf[1]));
      ret->value.tuple2.tag = tag;
    } else if(len >= 3) {
      CHKERR(object_new_tupleN(ret, len));
      memcpy(ret->value.tupleN.data, buf, len * sizeof(object_t *));
      ret->value.tupleN.tag = tag;
    }
    *out = ret;
    return EM_RESULT_OK;
  err:
    if(ret != nullptr) machine_return(m, ret);
    return errres;
  }
}

em_result
exec_ast_tuple(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  em_result errres = EM_RESULT_OK;
  stack_state_t state = MACHINE_STACK_STATE_DEFAULT;
  int len = 0;
  CHKERR(machine_get_stack_state(m, &state));
  CHKERR(exec_ast_tuple_list_t(m, &(v->value.tuple), &len));
  CHKERR(exec_ast_construct_tuple(m, nullptr, len, &(m->stack->value.tupleN.data[state]), &(out->value)));
 err: return errres;
}

em_result
exec_ast_funccall(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  object_t * callee = nullptr;
  int arglen = 0;
  stack_state_t state = MACHINE_STACK_STATE_DEFAULT;
  em_result errres = EM_RESULT_OK;
  variable_table_t * prev_vt = nullptr;
  object_t ** o = &(out->value);
  CHKERR(machine_get_stack_state(m, &state));
  // Evaluate Arguments.
  if(v->value.funccall.arguments.value != nullptr)
    CHKERR(exec_ast_tuple_list_t(m, &(v->value.funccall.arguments), &arglen));
  // Evaluate Callee.
  CHKERR(exec_ast(m, v->value.funccall.callee, &callee));
  TEST_AND_ERROR(!object_is_pointer(callee) || callee == nullptr || (object_kind(callee) != EMFRP_OBJECT_FUNCTION), EM_RESULT_TYPE_MISMATCH);
  CHKERR(machine_push(m, callee));
  // Execute.
  switch(callee->value.function.kind) {
  case EMFRP_PROGRAM_KIND_AST:
    out->kind = EXEC_RESULT_EXECUTE_FUNCTION;
    out->value = callee;
    for(int i = 0; i < arglen; ++i)
      CHKERR(machine_mark_gray(m, m->stack->value.stack.data[out->stack_state + i]));
    // May rewrite to memmove.
    memcpy(&(m->stack->value.stack.data[out->stack_state]), &(m->stack->value.stack.data[state]), arglen * sizeof(object_t *));
    out->arglen = arglen;
    break;
  case EMFRP_PROGRAM_KIND_NOTHING: break;
  case EMFRP_PROGRAM_KIND_CALLBACK: CHKERR(callee->value.function.function.callback(o, &(m->stack->value.tupleN.data[state]), arglen)); break;
  case EMFRP_PROGRAM_KIND_RECORD_CONSTRUCT:
    TEST_AND_ERROR(callee->value.function.function.construct.arity != arglen, EM_RESULT_INVALID_ARGUMENT);
    CHKERR(exec_ast_construct_tuple(m, callee->value.function.function.construct.tag, arglen, &(m->stack->value.tupleN.data[state]), o));
    break;
  case EMFRP_PROGRAM_KIND_RECORD_ACCESS: {
    object_t * t = m->stack->value.tupleN.data[state];
    size_t access_index = callee->value.function.function.access.index;
    object_t * tag = callee->value.function.function.access.tag;
    TEST_AND_ERROR(arglen != 1, EM_RESULT_INVALID_ARGUMENT);
    TEST_AND_ERROR(!object_is_pointer(t) || t == nullptr, EM_RESULT_TYPE_MISMATCH);
    switch(object_kind(t))  {
    case EMFRP_OBJECT_TUPLE1:
      TEST_AND_ERROR(!exec_equal(t->value.tuple1.tag, tag) || access_index >= 1, EM_RESULT_TYPE_MISMATCH);
      *o = t->value.tuple1.i0;
      break;
    case EMFRP_OBJECT_TUPLE2:
      TEST_AND_ERROR(!exec_equal(t->value.tuple2.tag, tag) || access_index >= 2, EM_RESULT_TYPE_MISMATCH);
      *o = access_index == 0 ? t->value.tuple2.i0 : t->value.tuple2.i1;
      break;
    case EMFRP_OBJECT_TUPLEN:
      TEST_AND_ERROR(!exec_equal(t->value.tupleN.tag, tag) || access_index >= t->value.tupleN.length, EM_RESULT_TYPE_MISMATCH);
      *o = object_tuple_ith(t, access_index);
      break;
    default: errres = EM_RESULT_TYPE_MISMATCH; goto err;
    }
    break;
  } 
  default: DEBUGBREAK; break;
  }
 err: return errres;
}

em_result
exec_ast_func(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  em_result errres = EM_RESULT_OK;
  CHKERR(machine_alloc(m, &(out->value)));
  return object_new_function_ast(out->value, machine_get_variable_table(m)->this_object_ref, v);
 err: return errres;
}

em_result
exec_ast_case(machine_t * m, parser_expression_t * v, exec_result_t * o) {
  em_result errres = EM_RESULT_OK;
  object_t * v_result = nullptr;
  CHKERR2(err2, exec_ast(m, v->value.caseof.of, &v_result));
  CHKERR2(err2, machine_push(m, v_result));
  parser_branch_list_t * bl = v->value.caseof.branches;
  for(;bl != nullptr; bl = bl->next) {
    switch(bl->deconstruct->kind) {
    case DECONSTRUCTOR_IDENTIFIER: case DECONSTRUCTOR_ANY: goto match_ok;
    case DECONSTRUCTOR_TUPLE:
      if(machine_test_match(m, bl->deconstruct->value.tuple.tag, bl->deconstruct->value.tuple.data, v_result) == EM_RESULT_OK)
	goto match_ok;
      break;
    case DECONSTRUCTOR_INTEGER:
      if(object_is_integer(v_result) && bl->deconstruct->value.integer == object_get_integer(v_result))
	goto match_ok;
      break;
#if EMFRP_ENABLE_FLOATING
    case DECONSTRUCTOR_FLOAT: break;
#endif
    default: DEBUGBREAK; break;
    }
    continue;
  match_ok: {
      CHKERR2(err2, machine_new_variable_table(m));
      CHKERR(machine_matches(m, bl->deconstruct, v_result));
      CHKERR(exec_ast_mono(m, bl->body, o));
    err: machine_pop_variable_table(m);
      return errres; // Leave this function.
    }
  }
  if(bl == nullptr) o->value = nullptr; // Nothing matches.
 err2: return errres;
}

em_result
exec_ast_begin(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  em_result errres = EM_RESULT_OK;
  object_t * o = nullptr;
  bool isVarTblCreated = false;
  parser_branch_list_t * bl = v->value.begin.branches;
  if(bl == nullptr) return EM_RESULT_OK;
  for(;bl->next != nullptr; bl = bl->next) {
    CHKERR(exec_ast(m, bl->body, &o));
    if(bl->deconstruct != nullptr) {
      CHKERR(machine_new_variable_table(m));
      isVarTblCreated = true;
      goto assign;
    }
  }
  goto last_turn;
  for(;bl->next != nullptr; bl = bl->next) {
    CHKERR(exec_ast(m, bl->body, &o));
    if(bl->deconstruct != nullptr) {
    assign:
      CHKERR(machine_matches(m, bl->deconstruct, o));
    }
  }
 last_turn:
  CHKERR(exec_ast_mono(m, bl->body, out));
 err: if(isVarTblCreated) machine_pop_variable_table(m);
  return errres;
}

em_result
exec_ast_identifier(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  return machine_lookup_variable(m, &(out->value), &(v->value.identifier)) ?
    EM_RESULT_OK : EM_RESULT_MISSING_IDENTIFIER;
}

em_result
exec_ast_lastidentifier(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  node_t * id;
  if(!machine_lookup_node(m, &id, &(v->value.identifier)))
    return EM_RESULT_MISSING_IDENTIFIER;
  out->value = id->last;
  return EM_RESULT_OK;
}

em_result
exec_ast_if(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  em_result errres = EM_RESULT_OK;
  object_t * cond_result = nullptr;
  CHKERR(exec_ast(m, v->value.ifthenelse.cond, &cond_result));
  if(cond_result == nullptr) {
    out->value = nullptr;
    return EM_RESULT_TYPE_MISMATCH;
  }
  return exec_ast_mono(m, cond_result != &object_false ? v->value.ifthenelse.then : v->value.ifthenelse.otherwise, out);
 err: return errres; 
}

em_result
exec_ast_execute_function(machine_t * m, object_t * v, int arglen, exec_result_t * out) {
  em_result errres = EM_RESULT_OK;
  variable_table_t * prev_vt = nullptr;
  switch(v->value.function.kind) {
  case EMFRP_PROGRAM_KIND_AST:
    TEST_AND_ERROR((arglen != 0) ^ (v->value.function.function.ast.program->value.function.arguments != nullptr), 
		   EM_RESULT_INVALID_ARGUMENT);
    prev_vt = machine_get_variable_table(m);
    CHKERR(machine_push(m, prev_vt->this_object_ref));
    if(v->value.function.function.ast.closure == nullptr) {
      CHKERR(machine_set_variable_table(m, nullptr)); // TODO: set the global scope.
    } else if(!object_is_pointer(v->value.function.function.ast.closure) ||
	      object_kind(v->value.function.function.ast.closure) != EMFRP_OBJECT_VARIABLE_TABLE) {
      errres = EM_RESULT_INVALID_ARGUMENT;
      goto err;
    } else
      CHKERR(machine_set_variable_table(m, v->value.function.function.ast.closure->value.variable_table.ptr));
    CHKERR2(err2, machine_new_variable_table(m));
    CHKERR2(err2, machine_match(m, v->value.function.function.ast.program->value.function.arguments,
				&(m->stack->value.stack.data[out->stack_state]), arglen));
    CHKERR2(err2, exec_ast_mono(m, v->value.function.function.ast.program->value.function.body, out));
    break;
  default: DEBUGBREAK;
  }
 err2: machine_set_variable_table(m, prev_vt);
 err:  return errres;
}

const executor bin_op_table[] = {
  exec_ast_addition, exec_ast_subtract, exec_ast_division, exec_ast_multiplication,
  exec_ast_modulo, exec_ast_left_shift, exec_ast_right_shift,
  exec_ast_less_or_equal, exec_ast_less_than, exec_ast_greater_or_equal, exec_ast_greater_than,
  exec_ast_equal, exec_ast_not_equal, exec_ast_and, exec_ast_or, exec_ast_xor, exec_ast_dand, exec_ast_dor
};

const executor op_table[] = {
  nullptr, nullptr, nullptr, exec_ast_identifier, exec_ast_lastidentifier, exec_ast_if, exec_ast_tuple,
  exec_ast_funccall, exec_ast_func, exec_ast_begin, exec_ast_case
};

em_result
exec_ast_mono(machine_t * m, parser_expression_t * v, exec_result_t * out) {
  if (EXPR_KIND_IS_INTEGER(v)) {
    return object_new_int(&(out->value), (int)((size_t)v >> 2));
  } else if (EXPR_KIND_IS_BOOLEAN(v)) {
    out->value = EXPR_IS_TRUE(v) ? &object_true : &object_false;
    return EM_RESULT_OK;
  } 
  else if (EXPR_KIND_IS_BIN_OP(v))
    return bin_op_table[v->kind >> PARSER_EXPRESSION_KIND_SHIFT](m, v, out);
  else
    return op_table[v->kind](m, v, out);
}

em_result
exec_ast(machine_t * m, parser_expression_t * v, object_t ** out) {
  em_result errres = EM_RESULT_OK;
  exec_result_t o = {.kind = EXEC_RESULT_OBJECT, .value = nullptr, .stack_state = MACHINE_STACK_STATE_DEFAULT, .arglen = 0};
  CHKERR2(err_state, machine_get_stack_state(m, &o.stack_state));
  CHKERR(exec_ast_mono(m, v, &o));
  while(o.kind == EXEC_RESULT_EXECUTE_FUNCTION) {
    m->stack->value.stack.length = o.stack_state + o.arglen;
    o.kind = EXEC_RESULT_OBJECT;
    CHKERR(exec_ast_execute_function(m, o.value, o.arglen, &o));
  }
  *out = o.value;
 err: machine_restore_stack_state(m, o.stack_state);
 err_state: return errres;
}
