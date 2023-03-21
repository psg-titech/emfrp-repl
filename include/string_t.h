/** -------------------------------------------
 * @file   string_t.h
 * @brief  Pascal String Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/2/21
 ------------------------------------------- */

#pragma once
#include "em_result.h"
#include "misc.h"
#include "emmem.h"
#include <string.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
// ! Pascal String
typedef struct string_t {
  // ! Buffer
  char_t * buffer;
  // ! Length of String.(Sometimes, Not actual size of buffer.)
  size_t length;
} string_t;

// ! zero clear for string_t
/* !
 * \param str String to be cleared.
 */
static inline void
string_null(string_t * str) {
  str->buffer = nullptr;
  str->length = 0;
}

// ! Freeing buffer.
/* !
 * \param str String to be freed.
 */
static inline void
string_free(string_t * str) {
  if(str->buffer != nullptr) em_free(str->buffer);
  string_null(str);
}

// ! Construct string_t.
/* !
 * \param out string_t to be constructed.
 * \param buffer The buffer. Not copied.
 * \param length Length of buffer.
 */
static inline void
string_new(string_t * out, char_t * buffer, const size_t length) {
  out->buffer = buffer;
  out->length = length;
}

// ! Java's CharAt.
/* !
 * \param str string_t to be referenced.
 * \param i Index
 */
static inline char_t
string_getAt(string_t * str, const int i) { return str->buffer[i]; }

// ! Construct string_t by C String.
/* !
 * \param out string_t to be constructed.
 * \param buffer The buffer. Not copied.
 */
static inline void string_new1(string_t * out, char_t * buffer) { string_new(out, buffer, em_strlen(buffer)); }

// ! Malloc and Construct string_t by C String.
/* !
 * \param buffer The buffer. Not copied.
 * \return Malloc-ed and constructed string_t.
 */
string_t * string_malloc_new(const char_t * buffer);

// ! Copy the string.
/* !
 * \param dst Destination
 * \param src Source
 */
em_result string_copy(string_t * dst, const string_t * src);

// ! Calculate the hash of given string.
/* !
 * \param self The string to be hashed.
 * \return Hash value
 */
size_t string_hash(const string_t * self);

// ! Compare given strings.
/* !
 * \param v1 The string
 * \param v2 The string
 * \return true if v1==v2.
 */
static inline bool string_compare(const string_t * v1, const string_t * v2) {
  return (v1->length == v2->length) && (0 == strncmp(v1->buffer, v2->buffer, v1->length));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
