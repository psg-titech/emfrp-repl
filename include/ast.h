/** -------------------------------------------
 * @file   ast.h
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/14
 ------------------------------------------- */

#pragma once
#include "emmem.h"
#include "string_t.h"
#include <stdint.h>
#include "collections/list_t.h"

struct parser_expression_t;

typedef struct parser_expression_tuple_list_t {
  struct parser_expression_t * value;
  struct parser_expression_tuple_list_t * next;
} parser_expression_tuple_list_t;

#define PARSER_EXPRESSION_KIND_SHIFT 3

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
  EXPR_KIND_ADDITION = (0 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Substraction(a - b)
  EXPR_KIND_SUBTRACTION = (1 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Division(a / b)
  EXPR_KIND_DIVISION = (2 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Multiplication(a * b)
  EXPR_KIND_MULTIPLICATION = (3 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Modulo (a % b)
  EXPR_KIND_MODULO = (4 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Left Shift (a << b)
  EXPR_KIND_LEFT_SHIFT = (5 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Right Shift (a >> b)
  EXPR_KIND_RIGHT_SHIFT = (6 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Less or Equal(a <= b)
  EXPR_KIND_LESS_OR_EQUAL = (7 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Less than(a < b)
  EXPR_KIND_LESS_THAN = (8 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Greater or Equal(a >= b)
  EXPR_KIND_GREATER_OR_EQUAL = (9 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Greater than(a > b)
  EXPR_KIND_GREATER_THAN = (10 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Equal(a = b)
  EXPR_KIND_EQUAL = (11 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Not Equal(a != b)
  EXPR_KIND_NOT_EQUAL = (12 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! And(a & b)
  EXPR_KIND_AND = (13 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Or(a | b)
  EXPR_KIND_OR = (14 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Xor(a ^ b)
  EXPR_KIND_XOR = (15 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Double And (a && b)
  EXPR_KIND_DAND = (16 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
  // ! Double Or (a || b)
  EXPR_KIND_DOR = (17 << PARSER_EXPRESSION_KIND_SHIFT) | 1,
    
  // ! Floating literal
  EXPR_KIND_FLOATING = 1 << PARSER_EXPRESSION_KIND_SHIFT,
  // ! Identifier
  EXPR_KIND_IDENTIFIER = 2 << PARSER_EXPRESSION_KIND_SHIFT,
  // ! Identifier @last
  EXPR_KIND_LAST_IDENTIFIER = 3 << PARSER_EXPRESSION_KIND_SHIFT,
  // ! If expression
  EXPR_KIND_IF = 4 << PARSER_EXPRESSION_KIND_SHIFT,
  // ! Tuple expression
  EXPR_KIND_TUPLE = 5 << PARSER_EXPRESSION_KIND_SHIFT,
  // ! Function call
  EXPR_KIND_FUNCCALL = 6 << PARSER_EXPRESSION_KIND_SHIFT,
  // ! Function
  EXPR_KIND_FUNCTION = 7 << PARSER_EXPRESSION_KIND_SHIFT,
} parser_expression_kind_t;

#define EXPR_KIND_IS_BIN_OP(expr) (((expr)->kind & 1) == 1)
#define EXPR_KIND_IS_INTEGER(expr) (((size_t)(expr) & 0x1) == 1)
#define EXPR_KIND_IS_BOOLEAN(expr) ((((size_t)(expr)) & ~0x6) == 0)
#define EXPR_IS_TRUE(expr) ((size_t)(expr) == 0x2)
#define EXPR_IS_FALSE(expr) ((size_t)(expr) == 0x6)

#define parser_expression_true() (parser_expression_t *)0x2;
#define parser_expression_false() (parser_expression_t *)0x6;

typedef struct string_or_tuple_t {
  bool isString;
  union {
    string_t * string;
    list_t /*<string_or_tuple_t>*/ * tuple;
  } value;
} string_or_tuple_t;

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
    // ! When kind is EXPR_KIND_TUPLE.
    parser_expression_tuple_list_t tuple;
    // ! When kind is EXPR_KIND_FUNCCALL
    struct {
      // Callee Expression
      struct parser_expression_t * callee;
      // Argument Expressions
      parser_expression_tuple_list_t arguments;
    } funccall;
    // ! When kind is EXPR_KIND_FUNCTION
    struct {
      // ! Reference Count
      size_t reference_count;
      // ! Argument List
      string_or_tuple_t * arguments;
      // ! Body
      struct parser_expression_t * body;
    } function;
  } value;
} parser_expression_t;

// ! Node Definition.
typedef struct parser_node_t {
  // ! Name of node.
  string_or_tuple_t name;
  // ! Expression of node.
  parser_expression_t * expression;
  // ! `init` Expression
  parser_expression_t * init_expression;
  // ! `as` Expression
  string_t * as;
} parser_node_t;

// ! Constructor of parser_node_t.
/* !
 * \param node_name Name of node. Not copied.
 * \param expression Expression of node. Not copied.
 * \param init_expression `init` expression. Not copied.
 * \return Malloc-ed and constructed parser_node_t
 */
static inline parser_node_t *
parser_node_new(string_or_tuple_t * node_name, parser_expression_t * expression, parser_expression_t * init_expression, string_t * node_as) {
  parser_node_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(parser_node_t)))
    return nullptr;
  ret->name = *node_name;
  em_free(node_name);
  ret->expression = expression;
  ret->init_expression = init_expression;
  ret->as = node_as;
  return ret;
}

// ! Shallow free for parser_node_t.
/* !
 * This does not free strings, and parser_node_t::expression.
 * This is used when adding a node succeeded.
 * \param pn To be freed.
 */
void parser_node_free_shallow(parser_node_t * pn);

// ! Deep free for parser_node_t.
/* !
 * This does free strings, and parser_node_t::expression.
 * This is used when adding a node failed.
 * \param pn To be freed.
 */
void parser_node_free_deep(parser_node_t * pn);


static inline string_or_tuple_t *
parser_node_identifier_new(string_t * str) {
  string_or_tuple_t * v;
  if(em_malloc((void **)&v, sizeof(string_or_tuple_t)))
    return nullptr;
  v->isString = true;
  v->value.string = str;
  return v;
}

static inline string_or_tuple_t *
parser_node_identifiers_new(list_t * li) {
  string_or_tuple_t * v;
  if(em_malloc((void **)&v, sizeof(string_or_tuple_t)))
    return nullptr;
  v->isString = false;
  v->value.tuple = li;
  return v;
}

static inline list_t /*<string_or_tuple_t>*/ *
parser_node_identifiers_prepend(string_or_tuple_t * head,
			    list_t /*<string_or_tuple_t>*/ * tail) {
  if(list_add2(&tail, string_or_tuple_t, head))
    return nullptr;
  return tail;
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
  em_free(ident);
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
  em_free(ident);
  return ret;
}

// ! Constructor of tuple expression.
/* !
 * \param e Inner expression.
 */
static inline parser_expression_t *
parser_expression_new_tuple(parser_expression_t * inner) {
  parser_expression_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(parser_expression_t)))
    return nullptr;
  ret->kind = EXPR_KIND_TUPLE;
  ret->value.tuple.next = nullptr;
  ret->value.tuple.value = inner;
  return ret;
}

// ! Prepend to the tuple expression.
/* !
 * \param self The tuple expression add to.
 * \param inner The expression to be added.
 */
static inline parser_expression_t *
parser_expression_tuple_prepend(parser_expression_t * self, parser_expression_t * inner) {
  #if DEBUG
  if(self->kind != EXPR_KIND_TUPLE)
    DEBUGBREAK;
  #endif
  parser_expression_tuple_list_t * new_tl = nullptr;
  if(em_malloc((void **)&new_tl, sizeof(parser_expression_tuple_list_t)))
    return nullptr;
  *new_tl= self->value.tuple;
  self->value.tuple.next = new_tl;
  self->value.tuple.value = inner;
  return self;
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
