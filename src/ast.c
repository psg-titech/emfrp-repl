/** -------------------------------------------
 * @file   ast.c
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/21
 ------------------------------------------- */

#include "ast.h"
#include "string_t.h"
#include <stdio.h>

const char * const binary_op_table[] = {
  "+", "-", "/", "*", "%", "<<", ">>", "<=", "<", ">=", ">", "=", "!=",
  "&", "|", "^", "&&", "||"
};

void
string_or_tuple_free_shallow(string_or_tuple_t * st) {
  if(!(st->isString))
    for(list_t * li = st->value.tuple; li != nullptr; ) {
      string_or_tuple_free_shallow((string_or_tuple_t*)(&(li->value)));
      list_t * ne = LIST_NEXT(li);
      free(li);
      li = ne;
    }
}

void
string_or_tuple_free_deep(string_or_tuple_t * st) {
  if(st->isString)
    string_free(st->value.string);
  else
    for(list_t * li = st->value.tuple; li != nullptr; ) {
      string_or_tuple_free_deep((string_or_tuple_t*)(&(li->value)));
      list_t * ne = LIST_NEXT(li);
      free(li);
      li = ne;
    }
}

void
parser_node_free_shallow(parser_node_t * pn) {
  string_or_tuple_free_shallow(&(pn->name));
  parser_expression_free(pn->init_expression);
  free(pn);
}

void
parser_node_free_deep(parser_node_t * pn) {
  string_or_tuple_free_deep(&(pn->name));
  if(pn->as != nullptr) {
    string_free(pn->as);
    em_free(pn->as);
  }
  parser_expression_free(pn->expression);
  if(pn->init_expression != nullptr) parser_expression_free(pn->init_expression);
  free(pn);
}

void
go_node_name_print(list_t * li) {
  printf("(");
  string_or_tuple_t * st = (string_or_tuple_t *)(&(li->value));
  if(st->isString)
    printf("%s", st->value.string->buffer);
  else
    go_node_name_print(st->value.tuple);
  
  for(li = LIST_NEXT(li); li != nullptr; li = LIST_NEXT(li)) {
    printf(", ");
    st = (string_or_tuple_t *)(&(li->value));
    if(st->isString)
      printf("%s", st->value.string->buffer);
    else
      go_node_name_print(st->value.tuple);
  }
  printf(")");
}

void
parser_node_print(parser_node_t * n) {
  if(n->name.isString) {
    printf("node %s = ", n->name.value.string->buffer);
  } else {
    printf("node ");
    go_node_name_print(n->name.value.tuple);
    if(n->as != nullptr)
      printf(" as %s", n->as->buffer);
    printf(" = ");
  }
  parser_expression_print(n->expression);
  printf("\n");
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
      //      go_node_name_print(e->value.function.arguments->tuple);
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
      expr->value.function.reference_count--;
      if(expr->value.function.reference_count > 0) return;
      parser_expression_free(expr->value.function.body);
      
      string_or_tuple_free_deep(expr->value.function.arguments);
      em_free(expr->value.function.arguments);
      break;
    }
    }
  free(expr);
}
