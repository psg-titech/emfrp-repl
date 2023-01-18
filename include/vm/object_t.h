/** -------------------------------------------
 * @file   object_t.h
 * @brief  Emfrp REPL object structure.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/18
 ------------------------------------------- */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "string_t.h"
#include "emmem.h"
#include "vm/program.h"
#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct variable_table_t;
// ! Kind of object_t.
/* !
   This distinguish the union, object_t::value.
   Assumption: malloc is aligned by 0x4.
   If the pointer of object_t is
     xxxxxx...xxx00 -> a pointer (distinguished by object_kind_t.)
     xxxxxx...xxx01 -> integer (immediate)
     xxxxxx...xxx10 -> reserved
     xxxxxx...xxx11 -> reserved
 */
#if __STD_VERSION__ <= 201710L
typedef enum object_kind_t {
#else
typedef enum object_kind_t : int32_t {
#endif
  // ! Free
  EMFRP_OBJECT_FREE = 0,
  // ! Tuple<T>
  EMFRP_OBJECT_TUPLE1 = 1 << 1,
  // ! Tuple<T1, T2>
  EMFRP_OBJECT_TUPLE2 = 2 << 1,
  // ! Tuple<T1, T2, T3, ...>
  EMFRP_OBJECT_TUPLEN = 3 << 1,
  // ! Symbol
  EMFRP_OBJECT_SYMBOL = 4 << 1,
  // ! String
  EMFRP_OBJECT_STRING = 5 << 1,
  // ! Function
  EMFRP_OBJECT_FUNCTION = 6 << 1,
  // ! Local varible table
  EMFRP_OBJECT_VARIABLE_TABLE = 7 << 1,
  // ! Local stack = TupleN
  EMFRP_OBJECT_STACK = 3 << 1,
} object_kind_t;

typedef enum emfrp_program_kind function_program_kind;
struct parser_expression_t;
// ! Object. (4 WORDS)
typedef struct object_t {
  // ! kind of value.
  object_kind_t kind;
  union {
    // ! used on Free.
    struct{ struct object_t * next; } free;
    // ! used on String.
    struct{ string_t value; } string;
    // ! used on Symbol.
    struct{ string_t value; } symbol;
    // ! used on tuple 1.
    struct{ struct object_t * i0; struct object_t * tag; } tuple1;
    // ! used on tuple 2.
    struct{ struct object_t * i0; struct object_t * i1; struct object_t * tag; } tuple2;
    // ! used on tuple N.
    struct{ struct object_t ** data; size_t length; struct object_t * tag; } tupleN;
    // ! used on Function.
    struct{
      // ! Kind of the function.
      function_program_kind kind;
      union {
	// ! Nothing
	nullptr_t nothing;
	// ! AST
	struct {
	  // ! Closure(kind == EMFRP_OBJECT_VARIABLE_TABLE)
	  struct object_t * closure; 
	  struct parser_expression_t * program;
	}ast;
	// ! CallBack
	foreign_func_t callback;
	// ! Record Constructor
	struct {
	  // ! Arity(length of tuple.)
	  size_t arity;
	  // ! The symbol of tag.
	  struct object_t * tag;
	} construct;
	// ! Record Accessor
	struct {
	  // ! Index(nth element).
	  size_t index;
	  // ! The symbol of tag.
	  struct object_t * tag;
	} access;
      } function;
    } function;
    // ! used on local variable table.
    struct{ struct variable_table_t * ptr; } variable_table;
    // ! used on local stack.
    struct{ struct object_t ** data; size_t length; struct object_t * capacity; } stack;
  } value;
} object_t;

// ! True Object
extern object_t object_true;
// ! False Object
extern object_t object_false;
#define object_is_pointer(v) (((size_t)v & 3) == 0)

#define object_kind(v) ((v)->kind & (-1 ^ 1))

// ! Mark the object.
/* !
 * \param self The object to be marked.
 */
static inline void
object_mark(object_t * self) {
  self->kind = self->kind | 1;
}

// ! Unmark the object.
/* !
 * \param o The object to be unmarked.
 */
static inline void
object_unmark(object_t * self) {
  self->kind = (self->kind & (int32_t)(-1 ^ 1));
}

// ! Test whether the object is marked.
/* !
 * \param o The object to be tested.
 */
static inline bool
object_is_marked(object_t * self) {
  return self->kind & 1;
}

// ! Create free object for garbage collections.
/* !
 * \param o The object to be renewed.
 * \param n The next object.
 */
static inline void
object_new_freelist(object_t * o, object_t * next) {
  o->kind = EMFRP_OBJECT_FREE;
  o->value.free.next = next;
}

// ! Test whether the object is an integer.
/* !
 * \param v The object to be tested.
 * \return Whether v is an integer.
 */
static inline bool object_is_integer(object_t * v) {
  return ((size_t)v&3) == 1;
}
// ! Test whether the object is a floating.
/* !
 * \param v The object to be tested.
 * \return Whether v is a floating.
 */
static inline bool object_is_floating(object_t * v) {
  return ((size_t)v&3) == 2;
}
// ! Test whether the object is a boolean value.
/* !
 * \param v The object to be tested.
 * \return Whether v is a boolean value.
 */
static inline bool object_is_boolean(object_t * v) {
  return v == &object_true || v == &object_false;
}

// ! Get the integer value from the given object.
/* !
 * \param v The integer object. Must be tested by object_is_integer.
 * \return The integer value.
 */
static inline int object_get_integer(object_t * v) {
#if DEBUG
  if(!object_is_integer(v)) {
    fprintf(stderr, "Error: Invalid objcet type.\n");
    return -1;
  }
#endif
  size_t ret = (size_t)v;
  return ((int)ret >> 2);
}


// ! Get the integer value from the given object.
/* !
 * \param v The integer object. Must be tested by object_is_integer.
 * \param out The result
 * \return The status
 */
static inline int object_get_int(object_t * v, int32_t * o) {
#if DEBUG
  if(!object_is_integer(v)) {
    return EM_RESULT_TYPE_MISMATCH;
  }
#endif
  size_t ret = (size_t)v;
  *o = ((int)ret >> 2);
  return EM_RESULT_OK;
}


// ! Freeing the given object.
/* !
 * \param v The object to be freed.
 */
static inline void object_free(object_t * v) {
  // if(((size_t)v & 3) == 0 && v != nullptr) em_free(v);
}

// DESIGN CONSIDERATION IS REQUIRED!
// ! Construct the new integer object.
/* !
 * \param out Output object.
 * \param v The value.
 * \return The result.
 */
static inline em_result object_new_int(object_t ** out, int v) {
  size_t * ret = (size_t *)out;
  *ret = (v << 2) | 1;
  return EM_RESULT_OK;
}

// ! Construct the new tuple1 object.
/* !
 * \param out The output object **Must be allocated before calling this function.**
 * \param v The value.
 * \return The result.
 */
static inline em_result
object_new_tuple1(object_t * out, object_t * v) {
  out->kind = EMFRP_OBJECT_TUPLE1 | (out->kind & 1);
  out->value.tuple1.i0 = v;
  return EM_RESULT_OK;
}

// ! Construct the new tuple2 object.
/* !
 * \param out The output object **Must be allocated before calling this function.**
 * \param v0 The value.
 * \param v1 The value.
 * \return The result.
 */
static inline em_result
object_new_tuple2(object_t * out, object_t * v0, object_t * v1) {
  out->kind = EMFRP_OBJECT_TUPLE2 | (out->kind & 1);
  out->value.tuple2.i0 = v0;
  out->value.tuple2.i1 = v1;
  return EM_RESULT_OK;
}

// ! Construct the new tupleN object.
/* !
 * \param out The output object **Must be allocated before calling this function.**
 * \param size Size of the object.
 * \return The result.
 */
static inline em_result
object_new_tupleN(object_t * out, size_t size) {
  out->kind = EMFRP_OBJECT_TUPLEN | (out->kind & 1);
  out->value.tupleN.length = size;
  out->value.tupleN.tag = nullptr;
  return em_allocarray((void **)(&(out->value.tupleN.data)), size, sizeof(object_t *));
}

// ! Construct the new symbol object.
/* !
 * \param out The output object **Must be allocated before calling this function.**
 * \param symbol The string of symbol.
 * \return The result.
 */
static inline em_result
object_new_symbol(object_t * out, string_t symbol) {
  out->kind = EMFRP_OBJECT_SYMBOL | (out->kind & 1);
  out->value.symbol.value = symbol;
  return EM_RESULT_OK;
}

// ! Construct the new function object(ast).
/* !
 * \param out The output object **Must be allocated before calling this function.**
 * \param closure The environment.
 * \param ast The program.
 * \return The result.
 */
static inline em_result
object_new_function_ast(object_t * out, object_t * closure, parser_expression_t * ast) {
  if(ast->kind != EXPR_KIND_FUNCTION) {
    DEBUGBREAK;
    return EM_RESULT_INVALID_ARGUMENT;
  }
  out->kind = EMFRP_OBJECT_FUNCTION | (out->kind & 1);
  out->value.function.function.ast.closure = closure;
  out->value.function.kind = EMFRP_PROGRAM_KIND_AST;
  out->value.function.function.ast.program = ast;
  ast->value.function.reference_count++;
  return EM_RESULT_OK;
}

static inline em_result
object_new_function_constructor(object_t * out, object_t * tag, size_t arity) {
  if(object_kind(tag) != EMFRP_OBJECT_SYMBOL) {
    DEBUGBREAK;
    return EM_RESULT_INVALID_ARGUMENT;
  }
  out->kind = EMFRP_OBJECT_FUNCTION | (out->kind & 1);
  out->value.function.kind = EMFRP_PROGRAM_KIND_RECORD_CONSTRUCT;
  out->value.function.function.construct.arity = arity;
  out->value.function.function.construct.tag = tag;
  return EM_RESULT_OK;
}


static inline em_result
object_new_function_accessor(object_t * out, object_t * tag, size_t index) {
  if(object_kind(tag) != EMFRP_OBJECT_SYMBOL) {
    DEBUGBREAK;
    return EM_RESULT_INVALID_ARGUMENT;
  }
  out->kind = EMFRP_OBJECT_FUNCTION | (out->kind & 1);
  out->value.function.kind = EMFRP_PROGRAM_KIND_RECORD_ACCESS;
  out->value.function.function.access.index = index;
  out->value.function.function.access.tag = tag;
  return EM_RESULT_OK;
}


// ! Construct the new vairbale table object.
/* !
 * \param out The output object **Must be allocated before calling this function.**
 * \param ptr The pointer to the table.
 * \return The result.
 */
static inline em_result
object_new_variable_table(object_t * out, struct variable_table_t * ptr) {
  out->kind = EMFRP_OBJECT_VARIABLE_TABLE | (out->kind & 1);
  out->value.variable_table.ptr = ptr;
  return EM_RESULT_OK;
}

// ! Construct the new stack.
/* !
 * \param out The output object **Must be allocated before calling this function.**
 * \param cap capacity.
 * \return The result.
 */
static inline em_result
object_new_stack(object_t * out, size_t cap) {
  em_result errres = EM_RESULT_OK;
  out->kind = EMFRP_OBJECT_STACK | (out->kind & 1);
  CHKERR(em_allocarray((void **)(&(out->value.stack.data)), cap, sizeof(object_t *)));
  CHKERR(object_new_int(&(out->value.stack.capacity), cap));
  out->value.stack.length = 0;
 err: return errres;
}

// Retrive ith of the given tuple.
#define object_tuple_ith(obj, ith) (((obj)->value.tupleN.data))[ith]

// ! Printing the object.
/* !
 * \param v The object to be printed.
 */
void object_print(object_t * v);

#ifdef __cplusplus
}
#endif /* __cplusplus */
