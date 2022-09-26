/** -------------------------------------------
 * @file   object_t.h
 * @brief  Emfrp REPL object structure.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/26
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
     xxxxxx...xxx10 -> floating point (immediate)
     xxxxxx...xxx11 -> boolean (immediate)
 */
#if __STD_VERSION__ <= 201710L
typedef enum object_kind_t {
#else
typedef enum object_kind_t : int32_t {
#endif
  // ! Invalid.
  EMFRP_OBJECT_INVALID = 0,
  // ! Symbol
  EMFRP_OBJECT_SYMBOL = 1,
  // ! String
  EMFRP_OBJECT_STRING = 2
} object_kind_t;

// ! Object.
typedef struct object_t {
  // ! kind of value.
  object_kind_t kind;
  union {
    // ! used on String.
    struct{ string_t value; } string;
    // ! used on Symbol.
    struct{ string_t value; } symbol;
  } value;
} object_t;

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
  return ((size_t)v&3) == 3;
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
  int32_t ret = (size_t)v;
  return ret >> 2;
}

// ! Freeing the given object.
/* !
 * \param v The object to be freed.
 */
static inline void object_free(object_t * v) {
  if(((size_t)v & 3) == 0) em_free(v);
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
