/** -------------------------------------------
 * @file   ast.h
 * @brief  Emfrp AST implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/24
 ------------------------------------------- */

#pragma once
#include "emmem.h"
#include "string_t.h"
#include <stdint.h>
#include "collections/list_t.h"
#if EMFRP_ENABLE_FLOATING
#include "hal/float.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  struct parser_expression_t;

  typedef struct parser_expression_tuple_list_t
  {
    struct parser_expression_t *            value;
    struct parser_expression_tuple_list_t * next;
  } parser_expression_tuple_list_t;

  typedef struct parser_branch_list_t
  {
    struct deconstructor_t *      deconstruct;
    struct parser_expression_t *  body;
    struct parser_branch_list_t * next;
  } parser_branch_list_t;

#define PARSER_EXPRESSION_KIND_SHIFT 4

// ! kind enum type of parser expression.
/* !
   This distinguish the union, parser_expression_t::value.
   If it is a binary expression, LSB is set.
   If it is a raw value, second bit from LSB is set.
 */
#if __STD_VERSION__ <= 201710L
  typedef enum parser_expression_kind_t
  {
#else
typedef enum parser_expression_kind_t
    : int32_t
{
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
    EXPR_KIND_FLOATING = 2,
    // ! Identifier
    EXPR_KIND_IDENTIFIER = 3,
    // ! Identifier @last
    EXPR_KIND_LAST_IDENTIFIER = 4,
    // ! If expression
    EXPR_KIND_IF = 5,
    // ! Tuple expression
    EXPR_KIND_TUPLE = 6,
    // ! Function call
    EXPR_KIND_FUNCCALL = 7,
    // ! Function
    EXPR_KIND_FUNCTION = 8,
    // ! { ..; ..; }
    EXPR_KIND_BEGIN = 9,
    // ! .. of:
    EXPR_KIND_CASE = 10,
  } parser_expression_kind_t;

#define EXPR_KIND_IS_BIN_OP(expr)  (((expr)->kind & ((1 << PARSER_EXPRESSION_KIND_SHIFT) - 1)) == 1)
#define EXPR_IS_POINTER(expr)      (((size_t)(expr)&0x3) == 0)
#define EXPR_KIND_IS_INTEGER(expr) (((size_t)(expr)&0x3) == 1)
#if EMFRP_ENABLE_FLOATING
#define EXPR_KIND_IS_FLOATING(expr) (((size_t)(expr)&0x3) == 2)
#endif
#define EXPR_KIND_IS_BOOLEAN(expr) ((((size_t)(expr)) & 0x3) == 3)
#define EXPR_IS_TRUE(expr)         ((size_t)(expr) == 0x7)
#define EXPR_IS_FALSE(expr)        ((size_t)(expr) == 0xB)

#define parser_expression_true()  (parser_expression_t *)0x7;
#define parser_expression_false() (parser_expression_t *)0xB;

  // ! Kind of deconstructor
  typedef enum deconstructor_kind
  {
    // ! Identifier
    DECONSTRUCTOR_IDENTIFIER,
    // ! Any
    DECONSTRUCTOR_ANY,
    // ! Tuple
    DECONSTRUCTOR_TUPLE,
    // ! Integer
    DECONSTRUCTOR_INTEGER
#if EMFRP_ENABLE_FLOATING
      // ! Floating
      ,
    DECONSTRUCTOR_FLOAT
#endif
  } deconstructor_kind;

  // ! Deconstructor Expression
  typedef struct deconstructor_t
  {
    deconstructor_kind kind;
    union
    {
      // ! Identifier
      string_t * identifier;
      // ! Tuple
      struct
      {
        // ! Tag(when tuple, tag is nullptr)
        string_t * tag;
        // ! Tuple
        list_t /*<deconstructor_t>*/ * data;
      } tuple;
      // ! Integer
      int integer;
#if EMFRP_ENABLE_FLOATING
      // ! Floating number.
      float floating;
#endif
    } value;
  } deconstructor_t;

  // ! Expression.(except for node definition.)
  typedef struct parser_expression_t
  {
    // ! kind of value.
    parser_expression_kind_t kind;
    union
    {
      // ! When kind & 1 == 1
      struct
      {
        // ! Left hand side
        struct parser_expression_t * lhs;
        // ! Right hand side
        struct parser_expression_t * rhs;
      } binary;
      // ! When kind & ?? = ??
      struct
      {
        // ! Primary Expression
        struct parser_expression_t * exp;
      } unary;
      // ! When kind is EXPR_KIND_IF.
      struct
      {
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
      struct
      {
        // Callee Expression
        struct parser_expression_t * callee;
        // Argument Expressions
        parser_expression_tuple_list_t arguments;
      } funccall;
      // ! When kind is EXPR_KIND_FUNCTION
      struct
      {
        // ! Reference Count
        size_t reference_count;
        // ! Argument List
        list_t /*<string_or_tuple_t>*/ * arguments;
        // ! Body
        struct parser_expression_t * body;
      } function;
      // ! When kind is EXPR_KIND_CASE
      struct
      {
        // ... of:
        struct parser_expression_t * of;
        // -> ...
        struct parser_branch_list_t * branches;
      } caseof;
      // ! When kind is EXPR_KIND_BEGIN
      struct
      {
        struct parser_branch_list_t * branches;
      } begin;
    } value;
  } parser_expression_t;

  // ! Node Definition.
  typedef struct parser_node_t
  {
    // ! Name of node.
    deconstructor_t name;
    // ! Expression of node.
    parser_expression_t * expression;
    // ! `init` Expression
    parser_expression_t * init_expression;
    // ! `as` Expression
    string_t * as;
  } parser_node_t;

  // ! Function Definition.
  typedef struct parser_func_t
  {
    // ! The name.
    string_t * name;
    // ! The arguments.
    list_t /*<deconstructor_t>*/ * arguments;
    // ! The body.
    parser_expression_t * expression;
  } parser_func_t;

  typedef struct parser_data_t
  {
    // ! The name.
    deconstructor_t name;
    // ! The body.
    parser_expression_t * expression;
  } parser_data_t;

  typedef struct parser_record_t
  {
    // ! The name.
    string_t name;
    // ! Accessors list.
    list_t /*<string_t>*/ * accessors;
  } parser_record_t;

  // ! Toplevel kind(parser_toplevel_t::kind)
  typedef enum parser_toplevel_kind
  {
    // ! parser_toplevel_t::value is parser_node_t.
    PARSER_TOPLEVEL_KIND_NODE,
    // ! parser_toplevel_t::value is parser_func_t.
    PARSER_TOPLEVEL_KIND_FUNC,
    // ! parser_toplevel_t::value is parser_data_t.
    PARSER_TOPLEVEL_KIND_DATA,
    // ! parser_toplevel_t::value is parser_expression_t.
    PARSER_TOPLEVEL_KIND_EXPR,
    // ! parser_toplevel_t::value is parser_record_t.
    PARSER_TOPLEVEL_KIND_RECORD
  } parser_toplevel_kind;

  // ! Toplevel Expression.
  typedef struct parser_toplevel_t
  {
    // ! Kind of value.
    parser_toplevel_kind kind;
    // ! The value.
    union
    {
      parser_node_t *       node;
      parser_func_t *       func;
      parser_data_t *       data;
      parser_expression_t * expression;
      parser_record_t *     record;
    } value;
  } parser_toplevel_t;

  // ! Constructor of parser_toplevel_t(node)
  /* !
 * \param n The node.
 * \return Malloc-ed and constructed parser_toplevel_t
 */
  static inline parser_toplevel_t *
  parser_toplevel_new_node(parser_node_t * n)
  {
    parser_toplevel_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_toplevel_t))) return nullptr;
    ret->kind       = PARSER_TOPLEVEL_KIND_NODE;
    ret->value.node = n;
    return ret;
  }

  // ! Constructor of parser_toplevel_t(func)
  /* !
 * \param f The function.
 * \return Malloc-ed and constructed parser_toplevel_t
 */
  static inline parser_toplevel_t *
  parser_toplevel_new_func(parser_func_t * f)
  {
    parser_toplevel_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_toplevel_t))) return nullptr;
    ret->kind       = PARSER_TOPLEVEL_KIND_FUNC;
    ret->value.func = f;
    return ret;
  }

  // ! Constructor of parser_toplevel_t(data)
  /* !
 * \param d The data definition.
 * \return Malloc-ed and constructed parser_toplevel_t
 */
  static inline parser_toplevel_t *
  parser_toplevel_new_data(parser_data_t * d)
  {
    parser_toplevel_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_toplevel_t))) return nullptr;
    ret->kind       = PARSER_TOPLEVEL_KIND_DATA;
    ret->value.data = d;
    return ret;
  }

  // ! Constructor of parser_toplevel_t(expr)
  /* !
 * \param e The expression.
 * \return Malloc-ed and constructed parser_toplevel_t
 */
  static inline parser_toplevel_t *
  parser_toplevel_new_expr(parser_expression_t * e)
  {
    parser_toplevel_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_toplevel_t))) return nullptr;
    ret->kind             = PARSER_TOPLEVEL_KIND_EXPR;
    ret->value.expression = e;
    return ret;
  }

  // ! Constructor of parser_toplevel_t(record)
  /* !
 * \param r The record.
 * \return Malloc-ed and constructed parser_toplevel_t
 */
  static inline parser_toplevel_t *
  parser_toplevel_new_record(parser_record_t * r)
  {
    parser_toplevel_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_toplevel_t))) return nullptr;
    ret->kind         = PARSER_TOPLEVEL_KIND_RECORD;
    ret->value.record = r;
    return ret;
  }

  // ! Shallow free for parser_node_t.
  /* !
 * This does not free strings, and expressions.
 * This is used when execution is succeeded.
 * \param pn To be freed.
 */
  void parser_toplevel_free_shallow(parser_toplevel_t * pt);

  // ! Deep free for parser_node_t.
  /* !
 * This does free strings, and expressions.
 * This is used when execution is failed.
 * \param pn To be freed.
 */
  void parser_toplevel_free_deep(parser_toplevel_t * pt);

  // ! Constructor of parser_func_t
  /* !
 * \param name The name.
 * \param dec The arguments.
 * \param e The body.
 * \return Malloc-ed and constructed parser_func_t
 */
  static inline parser_func_t *
  parser_func_new(string_t * name, list_t /*<deconstructor_t>*/ * dec, parser_expression_t * e)
  {
    parser_func_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_func_t))) return nullptr;
    ret->name       = name;
    ret->arguments  = dec;
    ret->expression = e;
    return ret;
  }

  // ! Shallow free for parser_func_t.
  /* !
 * This does not arguments and parser_func_t::expression.
 * This is used when adding a func succeeded.
 * \param pn To be freed.
 */
  void parser_func_free_shallow(parser_func_t * pf);

  // ! Deep free for parser_func_t.
  /* !
 * This does free arguments and parser_func_t::expression.
 * This is used when adding a func failed.
 * \param pn To be freed.
 */
  void parser_func_free_deep(parser_func_t * pf);

  // ! Constructor of parser_data_t
  /* !
 * \param dec The names.
 * \param e The body.
 * \return Malloc-ed and constructed parser_data_t
 */
  static inline parser_data_t *
  parser_data_new(deconstructor_t * dec, parser_expression_t * e)
  {
    parser_data_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_data_t))) return nullptr;
    ret->name = *dec;
    em_free(dec);
    ret->expression = e;
    return ret;
  }

  // ! Deep free for parser_data_t.
  /* !
 * This does free parser_data_t::expression.
 * This is used when adding a data failed.
 * \param pn To be freed.
 */
  void parser_data_free_deep(parser_data_t * pd);

  // ! Constructor of parser_record_t.
  /* !
 * \param name The name.
 * \param accessors The accessors list.
 * \return Malloc-ed and constructed parser_record_t
 */
  static inline parser_record_t *
  parser_record_new(string_t * name, list_t /*<string_t> */ * accessors)
  {
    parser_record_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_record_t))) return nullptr;
    ret->name = *name;
    em_free(name);
    ret->accessors = accessors;
    return ret;
  }

  // ! Deep free for parser_record_t.
  /* !
 * This is used when adding a record failed.
 * \param pr To be freed.
 */
  void parser_record_free_deep(parser_record_t * pr);

  // ! Constructor of parser_node_t.
  /* !
 * \param node_name Name of node. Not copied.
 * \param expression Expression of node. Not copied.
 * \param init_expression `init` expression. Not copied.
 * \return Malloc-ed and constructed parser_node_t
 */
  static inline parser_node_t *
  parser_node_new(
    deconstructor_t * node_name, parser_expression_t * expression,
    parser_expression_t * init_expression, string_t * node_as)
  {
    parser_node_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_node_t))) return nullptr;
    ret->name = *node_name;
    em_free(node_name);
    ret->expression      = expression;
    ret->init_expression = init_expression;
    ret->as              = node_as;
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

  static inline deconstructor_t *
  parser_deconstructor_new_identifier(string_t * str)
  {
    deconstructor_t * v;
    if(em_malloc((void **)&v, sizeof(deconstructor_t))) return nullptr;
    v->kind             = DECONSTRUCTOR_IDENTIFIER;
    v->value.identifier = str;
    return v;
  }

  static inline deconstructor_t *
  parser_deconstructor_new_tuple(string_t * tag, list_t /*<deconstructor_t>*/ * li)
  {
    deconstructor_t * v;
    if(em_malloc((void **)&v, sizeof(deconstructor_t))) return nullptr;
    v->kind             = DECONSTRUCTOR_TUPLE;
    v->value.tuple.tag  = tag;
    v->value.tuple.data = li;
    return v;
  }

  static inline deconstructor_t *
  parser_deconstructor_new_integer(int i)
  {
    deconstructor_t * v;
    if(em_malloc((void **)&v, sizeof(deconstructor_t))) return nullptr;
    v->kind          = DECONSTRUCTOR_INTEGER;
    v->value.integer = i;
    return v;
  }

  static inline list_t /*<deconstructor_t>*/ *
  parser_deconstructors_prepend(deconstructor_t * head, list_t /*<deconstructor_t>*/ * tail)
  {
    if(list_add2(&tail, deconstructor_t, head)) return nullptr;
    em_free(head);
    return tail;
  }

  static inline list_t /*<string_t> */ *
  parser_identifiers_prepend(string_t * str, list_t /*<string_t>*/ * tail)
  {
    if(list_add2(&tail, string_t, str)) return nullptr;
    em_free(str);
    return tail;
  }

  static inline parser_branch_list_t *
  parser_expression_branch_new(deconstructor_t * decon, parser_expression_t * body)
  {
    parser_branch_list_t * v;
    if(em_malloc((void **)&v, sizeof(parser_branch_list_t))) return nullptr;
    v->deconstruct = decon;
    v->body        = body;
    v->next        = nullptr;
    return v;
  }

  static inline parser_branch_list_t *
  parser_expression_branch_prepend(parser_branch_list_t * bt, parser_branch_list_t * next)
  {
    bt->next = next;
    return bt;
  }

  // ! Constructor of binary expression.
  /* !
 * \param lhs Left hand side. Not copied.
 * \param rhs Right hand side. Not copied.
 * \param kind The binary expression kind, validated only in Debug mode.
 * \return Malloc-ed and constructed parser_expression_t
 */
  static inline parser_expression_t *
  parser_expression_new_binary(
    parser_expression_t * lhs, parser_expression_t * rhs, parser_expression_kind_t kind)
  {
#if DEBUG
    if(kind & 1 == 0) DEBUGBREAK;
#endif
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t)) != EM_RESULT_OK) return nullptr;
    ret->kind             = kind;
    ret->value.binary.lhs = lhs;
    ret->value.binary.rhs = rhs;
    return ret;
  }

#define parser_expression_new_addition(lhs, rhs)                                                   \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_ADDITION)
#define parser_expression_new_subtraction(lhs, rhs)                                                \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_SUBTRACTION)
#define parser_expression_new_multiplication(lhs, rhs)                                             \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_MULTIPLICATION)
#define parser_expression_new_division(lhs, rhs)                                                   \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_DIVISION)
#define parser_expression_new_modulo(lhs, rhs)                                                     \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_MODULO)
#define parser_expression_new_left_shift(lhs, rhs)                                                 \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_LEFT_SHIFT)
#define parser_expression_new_right_shift(lhs, rhs)                                                \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_RIGHT_SHIFT)
#define parser_expression_new_less_or_equal(lhs, rhs)                                              \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_LESS_OR_EQUAL)
#define parser_expression_new_less_than(lhs, rhs)                                                  \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_LESS_THAN)
#define parser_expression_new_greater_or_equal(lhs, rhs)                                           \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_GREATER_OR_EQUAL)
#define parser_expression_new_greater_than(lhs, rhs)                                               \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_GREATER_THAN)
#define parser_expression_new_equal(lhs, rhs)                                                      \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_EQUAL)
#define parser_expression_new_not_equal(lhs, rhs)                                                  \
  parser_expression_new_binary(lhs, rhs, EXPR_KIND_NOT_EQUAL)
#define parser_expression_new_and(lhs, rhs)  parser_expression_new_binary(lhs, rhs, EXPR_KIND_AND)
#define parser_expression_new_or(lhs, rhs)   parser_expression_new_binary(lhs, rhs, EXPR_KIND_OR)
#define parser_expression_new_xor(lhs, rhs)  parser_expression_new_binary(lhs, rhs, EXPR_KIND_XOR)
#define parser_expression_new_dand(lhs, rhs) parser_expression_new_binary(lhs, rhs, EXPR_KIND_DAND)
#define parser_expression_new_dor(lhs, rhs)  parser_expression_new_binary(lhs, rhs, EXPR_KIND_DOR)

  static inline parser_expression_t *
  parser_expression_new_if(
    parser_expression_t * cond, parser_expression_t * then, parser_expression_t * otherwise)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
    ret->kind                       = EXPR_KIND_IF;
    ret->value.ifthenelse.cond      = cond;
    ret->value.ifthenelse.then      = then;
    ret->value.ifthenelse.otherwise = otherwise;
    return ret;
  }

  // ! Constructor of integer literal expression.
  /* !
 * \param num Value
 * \return Malloc-ed and constructed parser_expression_t
 */
  static inline parser_expression_t *
  parser_expression_new_integer(int num)
  {
    return (parser_expression_t *)(size_t)((num << 2) | 0x1);
  }

#if EMFRP_ENABLE_FLOATING
  static inline parser_expression_t *
  parser_expression_new_float(float f)
  {
    return (parser_expression_t *)(size_t)(inline_float(f) | 0x2);
  }
#endif

  // ! Constructor of indentifier expression.
  /* !
 * \param ident Identifier
 * \return Malloc-ed and constructed parser_expression_t
 */
  static inline parser_expression_t *
  parser_expression_new_identifier(string_t * ident)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
    ret->kind             = EXPR_KIND_IDENTIFIER;
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
  parser_expression_new_last_identifier(string_t * ident)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
    ret->kind             = EXPR_KIND_LAST_IDENTIFIER;
    ret->value.identifier = *ident;
    em_free(ident);
    return ret;
  }

  // ! Constructor of tuple expression.
  /* !
 * \param e Inner expression.
 */
  static inline parser_expression_t *
  parser_expression_new_tuple(parser_expression_t * inner)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
    ret->kind              = EXPR_KIND_TUPLE;
    ret->value.tuple.next  = nullptr;
    ret->value.tuple.value = inner;
    return ret;
  }

  // ! Prepend to the tuple expression.
  /* !
 * \param self The tuple expression add to.
 * \param inner The expression to be added.
 */
  static inline parser_expression_t *
  parser_expression_tuple_prepend(parser_expression_t * self, parser_expression_t * inner)
  {
#if DEBUG
    if(self->kind != EXPR_KIND_TUPLE) DEBUGBREAK;
#endif
    parser_expression_tuple_list_t * new_tl = nullptr;
    if(em_malloc((void **)&new_tl, sizeof(parser_expression_tuple_list_t))) return nullptr;
    *new_tl                 = self->value.tuple;
    self->value.tuple.next  = new_tl;
    self->value.tuple.value = inner;
    return self;
  }

  // ! Constructor of function expression.
  /* !
 * \param deconstructors The arguments deconstructor.
 * \param body The body of the function.
 * \return The result
 */
  static inline parser_expression_t *
  parser_expression_new_function(
    list_t /*<deconstructor_t>*/ * deconstructors, parser_expression_t * body)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
    ret->kind                           = EXPR_KIND_FUNCTION;
    ret->value.function.reference_count = 1;
    ret->value.function.arguments       = deconstructors;
    ret->value.function.body            = body;
    return ret;
  }

  // ! Constructor of function call.
  /* !
 * \param callee The callee
 * \param arguments The arguments.
 * \return The result
 */
  static inline parser_expression_t *
  parser_expression_new_function_call(parser_expression_t * callee, parser_expression_t * arguments)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
#if DEBUG
    if(arguments != nullptr && arguments->kind != EXPR_KIND_TUPLE) {
      DEBUGBREAK;  // NOT TUPLE
      return;
    }
#endif
    ret->kind                  = EXPR_KIND_FUNCCALL;
    ret->value.funccall.callee = callee;
    if(arguments == nullptr) {
      ret->value.funccall.arguments.value = nullptr;
    } else
      ret->value.funccall.arguments = arguments->value.tuple;
    em_free(arguments);
    return ret;
  }

  static inline parser_expression_t *
  parser_expression_new_case(parser_expression_t * v, parser_branch_list_t * branches)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
    ret->kind                  = EXPR_KIND_CASE;
    ret->value.caseof.of       = v;
    ret->value.caseof.branches = branches;
    return ret;
  }

  static inline parser_expression_t *
  parser_expression_new_begin(parser_branch_list_t * branches)
  {
    parser_expression_t * ret = nullptr;
    if(em_malloc((void **)&ret, sizeof(parser_expression_t))) return nullptr;
    ret->kind                 = EXPR_KIND_BEGIN;
    ret->value.begin.branches = branches;
    return ret;
  }

  // ! Freeing parser_expression_t.
  /* !
 * \param expr The expression to be freed.
 */
  void parser_expression_free(parser_expression_t * expr);

  // ! Used for parser_expression_print.
  extern const char * const binary_op_table[];

  // ! Print parser_toplevel_t to stdout.
  /* !
 * \ param t The toplevel expression to be printed.
 */
  void parser_toplevel_print(parser_toplevel_t * t);

  // ! Print parser_node_t to stdout.
  /* !
 * \param n The node definition to be printed.
 */
  void parser_node_print(parser_node_t * n);

  // ! Print parser_data_t to stdout.
  /* !
 * \param d The data definition to be printed.
 */
  void parser_data_print(parser_data_t * d);

  // ! Print parser_record_t to stdout.
  /* !
 * \param r The record definition to be printed.
 */
  void parser_record_print(parser_record_t * r);

  // ! Print parser_func_t to stdout.
  /* !
 * \param f The function definition to be printed.
 */
  void parser_function_print(parser_func_t * f);

  // ! Print parser_expression_t to stdout.
  /* !
 * \param n The expression to be printed.
 */
  void parser_expression_print(parser_expression_t * e);

#ifdef __cplusplus
}
#endif /* __cplusplus */
