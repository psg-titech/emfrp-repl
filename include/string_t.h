/** -------------------------------------------
 * @file   string_t.h
 * @brief  Pascal String Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */

#pragma once
#include "em_result.h"
#include "misc.h"
#include <string.h>

// ! Pascal String
typedef struct string_t {
  // ! Buffer
  char * buffer;
  // ! Length of String.(Sometimes, Not actual size of buffer.)
  size_t length;
} string_t;

// ! zero clear for string_t
/* !
 * \param str String to be cleared.
 */
void string_null(string_t * str);

// ! Freeing buffer.
/* !
 * \param str String to be freed.
 */
void string_free(string_t * str);

// ! Construct string_t.
/* !
 * \param out string_t to be constructed.
 * \param buffer The buffer. Not copied.
 * \param length Length of buffer.
 */
void string_new(string_t * out, char * buffer, const size_t length);

// ! Java's CharAt.
/* !
 * \param str string_t to be referenced.
 * \param i Index
 */
static inline char string_getAt(string_t * str, const int i) { return str->buffer[i]; }

// ! Construct string_t by C String.
/* !
 * \param out string_t to be constructed.
 * \param buffer The buffer. Not copied.
 */
static inline void string_new1(string_t * out, char * buffer) { string_new(out, buffer, strlen(buffer)); }

// ! Malloc and Construct string_t by C String.
/* !
 * \param buffer The buffer. Not copied.
 * \return Malloc-ed and constructed string_t.
 */
string_t * string_malloc_new(const char * buffer);

