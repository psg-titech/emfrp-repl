/** -------------------------------------------
 * @file   ast.c
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/22
 ------------------------------------------- */

#include "ast.h"
#include "string_t.h"
#include <stdio.h>

const char * const binary_op_table[] = {
  "+",
  "-",
  "/",
  "*"
};

void
parser_node_print(parser_node_t * n) {
  printf("node %s = ", n->name.buffer);
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
    printf(" %s ", binary_op_table[e->kind >> 2]);
    parser_expression_print(e->value.binary.rhs);
    printf(")");
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
    default: DEBUGBREAK; break;
    }
  }
}

void
parser_expression_free(parser_expression_t * expr) {
  if(EXPR_KIND_IS_INTEGER(expr)) return; // It is integer-typed.
  if(EXPR_KIND_IS_BOOLEAN(expr)) return; // It is boolean-typed.
  if(EXPR_KIND_IS_BIN_OP(expr)) {
    parser_expression_free(expr->value.binary.lhs);
    parser_expression_free(expr->value.binary.rhs);
  } else if(expr->kind == EXPR_KIND_IDENTIFIER || expr->kind == EXPR_KIND_LAST_IDENTIFIER)
    string_free(&(expr->value.identifier));
  free(expr);
}
