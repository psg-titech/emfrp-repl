/** -------------------------------------------
 * @file   ast.h
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/11/4
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
  // ! Modulo (a % b)
  EXPR_KIND_MODULO = 0x11,
  // ! Left Shift (a << b)
  EXPR_KIND_LEFT_SHIFT = 0x15,
  // ! Right Shift (a >> b)
  EXPR_KIND_RIGHT_SHIFT = 0x19,
  // ! Less or Equal(a <= b)
  EXPR_KIND_LESS_OR_EQUAL = 0x1D,
  // ! Less than(a < b)
  EXPR_KIND_LESS_THAN = 0x21,
  // ! Greater or Equal(a >= b)
  EXPR_KIND_GREATER_OR_EQUAL = 0x25,
  // ! Greater than(a > b)
  EXPR_KIND_GREATER_THAN = 0x29,
  // ! Equal(a = b)
  EXPR_KIND_EQUAL = 0x2D,
  // ! Not Equal(a != b)
  EXPR_KIND_NOT_EQUAL = 0x31,
  // ! And(a & b)
  EXPR_KIND_AND = 0x35,
  // ! Or(a | b)
  EXPR_KIND_OR = 0x39,
  // ! Xor(a ^ b)
  EXPR_KIND_XOR = 0x3D,
  // ! Double And (a && b)
  EXPR_KIND_DAND = 0x41,
  // ! Double Or (a || b)
  EXPR_KIND_DOR = 0x45,
    
  // ! Floating literal
  EXPR_KIND_FLOATING = 6,
  // ! Identifier
  EXPR_KIND_IDENTIFIER = 4,
  // ! Identifier @last
  EXPR_KIND_LAST_IDENTIFIER = 8,
  // ! If expression
  EXPR_KIND_IF = 12
} parser_expression_kind_t;

#define EXPR_KIND_IS_BIN_OP(expr) (((expr)->kind & 1) == 1)
#define EXPR_KIND_IS_INTEGER(expr) (((size_t)(expr) & 0x1) == 1)
#define EXPR_KIND_IS_BOOLEAN(expr) ((((size_t)(expr)) & ~0x6) == 0)
#define EXPR_IS_TRUE(expr) ((size_t)(expr) == 0x2)
#define EXPR_IS_FALSE(expr) ((size_t)(expr) == 0x6)

#define parser_expression_true() (parser_expression_t *)0x2;
#define parser_expression_false() (parser_expression_t *)0x6;

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
    // ! When kind is EXPR_KIND_IF.
    struct {
      // Condition
      struct parser_expression_t * cond;
      // Then
      struct parser_expression_t * then;
      // Else
      struct parser_expression_t * otherwise;
    } ifthenelse;
    // ! When kind is EXPR_KIND_FLOATING.
    float floating;
    // ! When kind is EXPR_KIND_IDENTIFIER or EXPR_KIND_LAST_IDENTIFIER.
    string_t identifier;
  } value;
} parser_expression_t;

// ! Node Definition.
typedef struct parser_node_t {
  // ! Name of node.
  string_t name;
  // ! Expression of node.
  parser_expression_t * expression;
  // ! `init` Expression
  parser_expression_t * init_expression;
} parser_node_t;

// ! Constructor of parser_node_t.
/* !
 * \param node_name Name of node. Not copied.
 * \param expression Expression of node. Not copied.
 * \param init_expression `init` expression. Not copied.
 * \return Malloc-ed and constructed parser_node_t
 */
static inline parser_node_t *
parser_node_new(string_t * node_name, parser_expression_t * expression, parser_expression_t * init_expression) {
  parser_node_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(parser_node_t)))
    return nullptr;
  ret->name = *node_name;
  ret->expression = expression;
  ret->init_expression = init_expression;
  return ret;
}

// ! Constructor of binary expression.
/* !
 * \param lhs Left hand side. Not copied.
 * \param rhs Right hand side. Not copied.
 * \param kind The binary expression kind, validated only in Debug mode.
 * \return Malloc-ed and constructed parser_expression_t
 */
static inline parser_expression_t *
parser_expression_new_binary(parser_expression_t * lhs, parser_expression_t * rhs, parser_expression_kind_t kind) {
#if DEBUG
  if(kind & 1 == 0) DEBUGBREAK;
#endif
  parser_expression_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(parser_expression_t)) != EM_RESULT_OK)
    return nullptr;
  ret->kind = kind;
  ret->value.binary.lhs = lhs;
  ret->value.binary.rhs = rhs;
  return ret;
}

#define parser_expression_new_addition(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_ADDITION)
#define parser_expression_new_subtraction(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_SUBTRACTION)
#define parser_expression_new_multiplication(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_MULTIPLICATION)
#define parser_expression_new_division(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_DIVISION)
#define parser_expression_new_modulo(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_MODULO)
#define parser_expression_new_left_shift(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_LEFT_SHIFT)
#define parser_expression_new_right_shift(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_RIGHT_SHIFT)
#define parser_expression_new_less_or_equal(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_LESS_OR_EQUAL)
#define parser_expression_new_less_than(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_LESS_THAN)
#define parser_expression_new_greater_or_equal(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_GREATER_OR_EQUAL)
#define parser_expression_new_greater_than(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_GREATER_THAN)
#define parser_expression_new_equal(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_EQUAL)
#define parser_expression_new_not_equal(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_NOT_EQUAL)
#define parser_expression_new_and(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_AND)
#define parser_expression_new_or(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_OR)
#define parser_expression_new_xor(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_XOR)
#define parser_expression_new_dand(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_DAND)
#define parser_expression_new_dor(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_DOR)

static inline parser_expression_t *
parser_expression_new_if(parser_expression_t * cond, parser_expression_t * then, parser_expression_t * otherwise) {
  parser_expression_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(parser_expression_t)))
    return nullptr;
  ret->kind = EXPR_KIND_IF;
  ret->value.ifthenelse.cond = cond;
  ret->value.ifthenelse.then = then;
  ret->value.ifthenelse.otherwise = otherwise;
  return ret;
}

// ! Constructor of integer literal expression.
/* !
 * \param num Value
 * \return Malloc-ed and constructed parser_expression_t
 */
static inline parser_expression_t *
parser_expression_new_integer(int32_t num) {
  return (parser_expression_t *)(size_t)((num << 1) | 0x1);
}

// ! Constructor of indentifier expression.
/* !
 * \param ident Identifier
 * \return Malloc-ed and constructed parser_expression_t
 */
static inline parser_expression_t *
parser_expression_new_identifier(string_t * ident) {
  parser_expression_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(parser_expression_t)))
    return nullptr;
  ret->kind = EXPR_KIND_IDENTIFIER;
  ret->value.identifier = *ident;
  return ret;
}

// ! Constructor of identifier expression(@last).
/* !
 * \param ident Identifier
 * \return Malloc-ed and constructed parser_expression_t
 */
static inline parser_expression_t *
parser_expression_new_last_identifier(string_t * ident) {
  parser_expression_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(parser_expression_t)))
    return nullptr;
  ret->kind = EXPR_KIND_LAST_IDENTIFIER;
  ret->value.identifier = *ident;
  return ret;
}

// ! Freeing parser_expression_t.
/* !
 * \param expr The expression to be freed.
 */
void
parser_expression_free(parser_expression_t * expr);

// ! Used for parser_expression_print.
extern const char * const binary_op_table[];

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
