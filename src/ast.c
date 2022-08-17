/** -------------------------------------------
 * @file   ast.c
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */

#include "ast.h"
#include "string_t.h"
#include <stdio.h>

void
parser_node_print(parser_node_t * n) {
  printf("node %s = ", n->name.buffer);
  parser_expression_print(n->expression);
  printf("\n");
}

void
parser_expression_print(parser_expression_t * e) {
  if((e->kind & 1) == 1) {
    printf("(");
    parser_expression_print(e->value.binary.lhs);
    printf(" %s ", binary_op_table[e->kind]);
    parser_expression_print(e->value.binary.rhs);
    printf(")");
  } else {
    switch(e->kind) {
    case EXPR_KIND_FLOATING:
      printf("%f", e->value.floating); break;
    case EXPR_KIND_INTEGER:
      printf("%d", e->value.integer); break;
    case EXPR_KIND_IDENTIFIER:
      printf("%s", e->value.identifier.buffer); break;
    }
  }
}
