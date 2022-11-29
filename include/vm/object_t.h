/** -------------------------------------------
 * @file   object_t.h
 * @brief  Emfrp REPL object structure.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/11/29
 ------------------------------------------- */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "string_t.h"
#include "emmem.h"
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
  EMFRP_OBJECT_STRING = 5 << 1
} object_kind_t;

// ! Object.
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
    struct{ struct object_t * i0; } tuple1;
    // ! used on tuple 2.
    struct{ struct object_t * i0; struct object_t * i1; } tuple2;
    // ! used on tuple N.
    struct{ size_t length; struct object_t ** data; } tupleN;
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
static inline int32_t object_get_integer(object_t * v) {
#if DEBUG
  if(!object_is_integer(v)) {
    fprintf(stderr, "Error: Invalid objcet type.\n");
    return -1;
  }
#endif
  size_t ret = (size_t)v;
  return (int32_t)(ret >> 2);
}

// ! Freeing the given object.
/* !
 * \param v The object to be freed.
 */
static inline void object_free(object_t * v) {
  // if(((size_t)v & 3) == 0 && v != nullptr) em_free(v);
}

// ! Construct the new integer object.
/* !
 * \param out Output object.
 * \param v The value.
 * \return The result.
 */
static inline em_result object_new_int(object_t ** out, int32_t v) {
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
  return em_malloc((void **)(&(out->value.tupleN.data)), sizeof(object_t *) * size);
}

// Retrive ith of the given tuple.
#define object_tuple_ith(obj, ith) (((obj)->value.tupleN.data))[ith]

// ! Printing the object.
/* !
 * \param v The object to be printed.
 */
void object_print(object_t * v);
