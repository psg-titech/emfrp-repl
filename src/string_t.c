/** -------------------------------------------
 * @file   string_t.c
 * @brief  Pascal String Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/11/4
 ------------------------------------------- */

#include "string_t.h"
#include "emmem.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
void
string_null(string_t * str) {
  str->buffer = nullptr;
  str->length = 0;
}

void
string_free(string_t * str) {
  if(str->buffer != nullptr) em_free(str->buffer);
  str->buffer = nullptr;
  str->length = 0;
}

void
string_new(string_t * out, char_t * buffer, const size_t length) {
  out->buffer = buffer;
  out->length = length;
}

string_t *
string_malloc_new(const char_t * buffer) {
  string_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(string_t)) != EM_RESULT_OK)
    return nullptr;
  ret->length = em_strlen(buffer);
  ret->buffer = em_strdup(buffer);
  return ret;
}

em_result
string_copy(string_t * dst, const string_t * src) {
  dst->length = src->length;
  dst->buffer = em_strdup(src->buffer);
  return EM_RESULT_OK;
}


size_t
string_hash(const string_t * self) {
  size_t ret = 0;
  for(int i = 0; i < self->length; ++i)
    ret ^= self->buffer[i];
  return ret;
}
