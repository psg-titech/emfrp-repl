/** -------------------------------------------
 * @file   ast.h
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */

#pragma once
#include "emmem.h"
#include "string_t.h"
#include <stdint.h>

// ! kind enum type of parser expression.
/* !
   This distinguish the union, parser_expression_t::value.
   If it is a binary expression, LSB is set.
   If it is a raw value, second bit from LSB is set.
 */
#if __STD_VERSION__ <= 201710L
typedef enum parser_expression_kind_t {
#else
typedef enum parser_expression_kind_t : int32_t {
#endif
  // & 1 == 1 => BINARY EXPRESSION.
  // & 2 == 2 => RAW VALUE.
  // ! Null(Illegal)
  EXPR_KIND_NULL = 0,
  // ! Addition(a + b)
  EXPR_KIND_ADDITION = 1,
  // ! Substraction(a - b)
  EXPR_KIND_SUBTRACTION = 5,
  // ! Division(a / b)
  EXPR_KIND_DIVISION = 9,
  // ! Multiplication(a * b)
  EXPR_KIND_MULTIPLICATION = 13,
  // ! Integer literal
  EXPR_KIND_INTEGER = 2,
  // ! Floating literal
  EXPR_KIND_FLOATING = 6,
  // ! Identifier
  EXPR_KIND_IDENTIFIER = 4
} parser_expression_kind_t;

// ! Expression.(except for node definition.)
typedef struct parser_expression_t {
  // ! kind of value.
  parser_expression_kind_t kind;
  union {
    // ! When kind & 1 == 1
    struct {
      // ! Left hand side
      struct parser_expression_t * lhs;
      // ! Right hand side
      struct parser_expression_t * rhs;
    } binary;
    // ! When kind & ?? = ??
    struct {
      // ! Primary Expression
      struct parser_expression_t * exp;
    } unary;
    // ! When kind is EXPR_KIND_INTEGER.
    int32_t integer;
    // ! When kind is EXPR_KIND_FLOATING.
    float floating;
    // ! When kind is EXPR_KIND_IDENTIFIER.
    string_t identifier;
  } value;
} parser_expression_t;

// ! Node Definition.
typedef struct parser_node_t {
  // ! Name of node.
  string_t name;
  // ! Expression of node.
  parser_expression_t * expression;
} parser_node_t;

// ! Constructor of parser_node_t.
/* !
 * \param node_name Name of node. Not copied.
 * \param expression Expression of node. Not copied.
 * \return Malloc-ed and constructed parser_node_t
 */
static inline parser_node_t *
parser_node_new(string_t * node_name, parser_expression_t * expression) {
  parser_node_t * ret = (parser_node_t *)em_malloc(sizeof(parser_node_t));
  ret->name = *node_name;
  ret->expression = expression;
  return ret;
}

// ! Constructor of addition expression.
/* !
 * \param lhs Left hand side. Not copied.
 * \param rhs Right hand side. Not copied.
 * \return Malloc-ed and constructed parser_expression_t
 */
static inline parser_expression_t *
parser_expression_new_addition(parser_expression_t * lhs, parser_expression_t * rhs) {
  parser_expression_t * ret = (parser_expression_t *)em_malloc(sizeof(parser_expression_t));
  ret->kind = EXPR_KIND_ADDITION;
  ret->value.binary.lhs = lhs;
  ret->value.binary.rhs = rhs;
  return ret;
}

// ! Constructor of integer literal expression.
/* !
 * \param num Value
 * \return Malloc-ed and constructed parser_expression_t
 */
static inline parser_expression_t *
parser_expression_new_integer(int32_t num) {
  parser_expression_t * ret = (parser_expression_t *)em_malloc(sizeof(parser_expression_t));
  ret->kind = EXPR_KIND_INTEGER;
  ret->value.integer = num;
  return ret;
}

// ! Constructor of indentifier expression.
/* !
 * \param ident Identifier
 * \return Malloc-ed and constructed parser_expression_t
 */
static inline parser_expression_t *
parser_expression_new_identifier(string_t * ident) {
  parser_expression_t * ret = (parser_expression_t *)em_malloc(sizeof(parser_expression_t));
  ret->kind = EXPR_KIND_IDENTIFIER;
  ret->value.identifier = *ident;
  return ret;
}

// ! Used for parser_expression_print.
static char * binary_op_table[] = {
  "",
  "+",
  "",
  "-",
  "",
  "/",
  "",
  "*"
};

// ! Print parser_node_t to stdout.
/* !
 * \param n The node definition to be printed.
 */
void
parser_node_print(parser_node_t * n);

// ! Print parser_expression_t to stdout.
/* !
 * \param n The expression to be printed.
 */
void
parser_expression_print(parser_expression_t * e);
