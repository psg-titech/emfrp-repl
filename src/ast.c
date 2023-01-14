/** -------------------------------------------
 * @file   ast.c
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/14
 ------------------------------------------- */

#include "ast.h"
#include "string_t.h"
#include <stdio.h>

const char * const binary_op_table[] = {
  "+", "-", "/", "*", "%", "<<", ">>", "<=", "<", ">=", ">", "=", "!=",
  "&", "|", "^", "&&", "||"
};

void
deconstructor_free_shallow(deconstructor_t * st) {
  switch(st->kind) {
  case DECONSTRUCTOR_IDENTIFIER: break;
  case DECONSTRUCTOR_TUPLE:
    if(st->value.tuple.tag != nullptr) { // ILLEGAL STATE
      DEBUGBREAK;
      string_free(st->value.tuple.tag);
      em_free(st->value.tuple.tag);
    }
    for(list_t * li = st->value.tuple.data; li != nullptr; ) {
      deconstructor_free_shallow((deconstructor_t*)(&(li->value)));
      list_t * ne = LIST_NEXT(li);
      free(li);
      li = ne;
    }
    break;
  default: break;
  }
}

void
deconstructor_free_deep(deconstructor_t * st) {
  switch(st->kind) {
  case DECONSTRUCTOR_IDENTIFIER:
    string_free(st->value.identifier);
    em_free(st->value.identifier);
    break;
  case DECONSTRUCTOR_TUPLE:
    if(st->value.tuple.tag != nullptr) {
      string_free(st->value.tuple.tag);
      em_free(st->value.tuple.tag);
    }
    for(list_t * li = st->value.tuple.data; li != nullptr; ) {
      deconstructor_free_deep((deconstructor_t*)(&(li->value)));
      list_t * ne = LIST_NEXT(li);
      free(li);
      li = ne;
    }
    break;
  default: break;
  }
}

void
parser_toplevel_free_shallow(parser_toplevel_t * pt) {
  switch(pt->kind) {
  case PARSER_TOPLEVEL_KIND_NODE: parser_node_free_shallow(pt->value.node); break;
  case PARSER_TOPLEVEL_KIND_DATA: parser_data_free_deep(pt->value.data); break;
  case PARSER_TOPLEVEL_KIND_FUNC: parser_func_free_shallow(pt->value.func); break;
  case PARSER_TOPLEVEL_KIND_EXPR: parser_expression_free(pt->value.expression); break;
  case PARSER_TOPLEVEL_KIND_RECORD: parser_record_free_deep(pt->value.record); break;
  }
  em_free(pt);
}

void
parser_toplevel_free_deep(parser_toplevel_t * pt) {
  switch(pt->kind) {
  case PARSER_TOPLEVEL_KIND_NODE: parser_node_free_deep(pt->value.node); break;
  case PARSER_TOPLEVEL_KIND_DATA: parser_data_free_deep(pt->value.data); break;
  case PARSER_TOPLEVEL_KIND_FUNC: parser_func_free_deep(pt->value.func); break;
  case PARSER_TOPLEVEL_KIND_EXPR: parser_expression_free(pt->value.expression); break;
  case PARSER_TOPLEVEL_KIND_RECORD: parser_record_free_deep(pt->value.record); break;
  }
  em_free(pt);
}

void
parser_func_free_shallow(parser_func_t * pf) {
  string_free(pf->name);
  em_free(pf);
}

void
parser_func_free_deep(parser_func_t * pf) {
  string_free(pf->name);
  em_free(pf->name);
  if(pf->arguments != nullptr) {
    deconstructor_t dt = {DECONSTRUCTOR_TUPLE, .value.tuple.tag = nullptr, .value.tuple.data = pf->arguments};
    deconstructor_free_deep(&dt);
  }
  parser_expression_free(pf->expression);
  em_free(pf);
}

void
parser_data_free_deep(parser_data_t * pd) {
  deconstructor_free_deep(&(pd->name));
  parser_expression_free(pd->expression);
  em_free(pd);
}

void
parser_record_free_deep(parser_record_t * pr) {
  string_free(&(pr->name));
  list_t /*<string_t>*/ * li = pr->accessors;
  while(li != nullptr) {
    list_t * ne = LIST_NEXT(li);
    string_t * v = (string_t *)(&(li->value));
    string_free(v);
    em_free(li);
    li = ne;
  }
  em_free(pr);
}

void
parser_node_free_shallow(parser_node_t * pn) {
  deconstructor_free_shallow(&(pn->name));
  parser_expression_free(pn->init_expression);
  em_free(pn);
}

void
parser_node_free_deep(parser_node_t * pn) {
  deconstructor_free_deep(&(pn->name));
  if(pn->as != nullptr) {
    string_free(pn->as);
    em_free(pn->as);
  }
  parser_expression_free(pn->expression);
  if(pn->init_expression != nullptr) parser_expression_free(pn->init_expression);
  em_free(pn);
}

void
go_deconstructor_print(list_t /*<deconstructor_t>*/ * li) {
  if(li == nullptr) return;
  printf("(");
  deconstructor_t * dt = (deconstructor_t *)(&(li->value));
  switch(dt->kind) {
  case DECONSTRUCTOR_IDENTIFIER:
    printf("%s", dt->value.identifier->buffer);
    break;
  case DECONSTRUCTOR_TUPLE:
    if(dt->value.tuple.tag != nullptr)
      printf("%s ", dt->value.tuple.tag->buffer);
    go_deconstructor_print(dt->value.tuple.data);
    break;
  case DECONSTRUCTOR_INTEGER:
    printf("%d", dt->value.integer);
    break;
#if EMFRP_ENABLE_FLOATING
  case DECONSTRUCTOR_FLOATING:
    printf("%lf", dt->value.floating);
    break;
#endif
  }
  for(li = LIST_NEXT(li); li != nullptr; li = LIST_NEXT(li)) {
    printf(", ");
    dt = (deconstructor_t *)(&(li->value));
    
    switch(dt->kind) {
    case DECONSTRUCTOR_IDENTIFIER:
      printf("%s", dt->value.identifier->buffer);
      break;
    case DECONSTRUCTOR_TUPLE:
      if(dt->value.tuple.tag != nullptr)
	printf("%s ", dt->value.tuple.tag->buffer);
      go_deconstructor_print(dt->value.tuple.data);
    break;
    case DECONSTRUCTOR_INTEGER:
      printf("%d", dt->value.integer);
      break;
#if EMFRP_ENABLE_FLOATING
    case DECONSTRUCTOR_FLOATING:
      printf("%lf", dt->value.floating);
      break;
#endif
    }
  }
  printf(")");
}

void
parser_toplevel_print(parser_toplevel_t * t) {
  switch(t->kind) {
  case PARSER_TOPLEVEL_KIND_NODE: parser_node_print(t->value.node); break;
  case PARSER_TOPLEVEL_KIND_DATA: parser_data_print(t->value.data); break;
  case PARSER_TOPLEVEL_KIND_FUNC: parser_function_print(t->value.func); break;
  case PARSER_TOPLEVEL_KIND_EXPR: parser_expression_print(t->value.expression); break;
  case PARSER_TOPLEVEL_KIND_RECORD: parser_record_print(t->value.record); break;
  default: DEBUGBREAK; break;
  }
}

void
parser_node_print(parser_node_t * n) {
  switch(n->name.kind) {
  case DECONSTRUCTOR_IDENTIFIER:
    printf("node %s = ", n->name.value.identifier->buffer);
    break;
  case DECONSTRUCTOR_TUPLE:
    printf("node ");
    go_deconstructor_print(n->name.value.tuple.data);
    printf(" = ");
    break;
  default: DEBUGBREAK; break;
  }
  parser_expression_print(n->expression);
  printf("\n");
}

void
parser_data_print(parser_data_t * d) {
  switch(d->name.kind) {
  case DECONSTRUCTOR_IDENTIFIER:
    printf("data %s = ", d->name.value.identifier->buffer);
    break;
  case DECONSTRUCTOR_TUPLE:
    printf("data ");
    go_deconstructor_print(d->name.value.tuple.data);
    printf(" = ");
    break;
  default: DEBUGBREAK; break;
  }
  parser_expression_print(d->expression);
}

void
parser_function_print(parser_func_t * f) {
  printf("func %s", f->name->buffer);
  go_deconstructor_print(f->arguments);
  printf(" = ");
  parser_expression_print(f->expression);
}

void
parser_record_print(parser_record_t * r) {
  printf("record %s(", r->name.buffer);
  list_t /*<string_t>*/ * li = r->accessors;
  while(li != nullptr) {
    string_t * v = (string_t *)(&(li->value));
    printf(LIST_NEXT(li) == nullptr ? "%s" : "%s, ", v->buffer);
    li = LIST_NEXT(li);
  }
  printf(")");
}

void
parser_expression_print(parser_expression_t * e) {
  if(EXPR_KIND_IS_INTEGER(e))
    printf("%d", (int32_t)((size_t)e>>1));
  else if(EXPR_KIND_IS_BOOLEAN(e))
    printf(EXPR_IS_TRUE(e) ? "true" : "false");
  else if((e->kind & 1) == 1) {
    printf("(");
    parser_expression_print(e->value.binary.lhs);
    printf(" %s ", binary_op_table[e->kind >> PARSER_EXPRESSION_KIND_SHIFT]);
    parser_expression_print(e->value.binary.rhs);
    printf(")");
  } else if (e == nullptr) {
    printf("NIL");
  } else {
    switch(e->kind) {
    case EXPR_KIND_FLOATING:
      printf("%f", e->value.floating); break;
    case EXPR_KIND_IDENTIFIER:
      printf("%s", e->value.identifier.buffer); break;
    case EXPR_KIND_LAST_IDENTIFIER:
      printf("%s@last", e->value.identifier.buffer); break;
    case EXPR_KIND_IF:
      printf("if ");
      parser_expression_print(e->value.ifthenelse.cond);
      printf(" then ");
      parser_expression_print(e->value.ifthenelse.then);
      printf(" else ");
      parser_expression_print(e->value.ifthenelse.otherwise);
      break;
    case EXPR_KIND_TUPLE:
      printf("(");
      parser_expression_tuple_list_t * tl = &(e->value.tuple);
      while(tl != nullptr) {
        parser_expression_print(tl->value);
        if(tl->next != nullptr)
          printf(", ");
        tl = tl->next;
      }
      printf(")");
      break;
    case EXPR_KIND_FUNCCALL:
      printf("(");
      parser_expression_print(e->value.funccall.callee);
      printf(")");
      {
        printf("(");
        parser_expression_tuple_list_t * tl = &(e->value.funccall.arguments);
        if(tl->value != nullptr) 
        while(tl != nullptr) {
          parser_expression_print(tl->value);
          if(tl->next != nullptr)
            printf(", ");
          tl = tl->next;
        }
	printf(")");
      }
      break;
    case EXPR_KIND_FUNCTION: {
      printf("(fun");
      go_deconstructor_print(e->value.function.arguments);
      printf(" -> (");
      parser_expression_print(e->value.function.body);
      printf("))");
      break;
    }
    default: DEBUGBREAK; break;
    }
  }
}

void
parser_expression_free(parser_expression_t * expr) {
  if(EXPR_KIND_IS_INTEGER(expr)) return; // It is integer-typed.
  if(EXPR_KIND_IS_BOOLEAN(expr)) return; // It is boolean-typed.
  if(expr == nullptr) return;
  if(EXPR_KIND_IS_BIN_OP(expr)) {
    parser_expression_free(expr->value.binary.lhs);
    parser_expression_free(expr->value.binary.rhs);
  } else
    switch(expr->kind) {
    case EXPR_KIND_IDENTIFIER:
    case EXPR_KIND_LAST_IDENTIFIER:
      string_free(&(expr->value.identifier)); break;
    case EXPR_KIND_IF:
      parser_expression_free(expr->value.ifthenelse.cond);
      parser_expression_free(expr->value.ifthenelse.then);
      parser_expression_free(expr->value.ifthenelse.otherwise);
      break;
    case EXPR_KIND_TUPLE: {
      parser_expression_tuple_list_t * tl = &expr->value.tuple;
      parser_expression_free(tl->value);
      tl = tl->next;
      while(tl != nullptr) {
	parser_expression_tuple_list_t * v = tl->next;
	parser_expression_free(tl->value);
	free(tl);
	tl = v;
      }
      break;
    }
    case EXPR_KIND_FUNCCALL: {
      parser_expression_tuple_list_t * tl = &expr->value.funccall.arguments;
      tl = tl->next;
      while(tl != nullptr) {
        parser_expression_tuple_list_t * v = tl->next;
        parser_expression_free(tl->value);
        free(tl);
        tl = v;
      }
      parser_expression_free(expr->value.funccall.callee);
      break;
    }
    case EXPR_KIND_FUNCTION: {
      deconstructor_t dt = {DECONSTRUCTOR_TUPLE, .value.tuple.tag = nullptr, .value.tuple.data = expr->value.function.arguments};
      expr->value.function.reference_count--;
      if(expr->value.function.reference_count > 0) return;
      parser_expression_free(expr->value.function.body);
      
      deconstructor_free_deep(&dt);
      em_free(expr->value.function.arguments);
      break;
    }
    default: break;
    }
  free(expr);
}
