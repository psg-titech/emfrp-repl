/** -------------------------------------------
 * @file   string_t.c
 * @brief  Pascal String Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/28
 ------------------------------------------- */

#include "string_t.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

string_t *
string_malloc_new(const char_t * buffer)
{
  string_t * ret = nullptr;
  if(em_malloc((void **)&ret, sizeof(string_t)) != EM_RESULT_OK) return nullptr;
  ret->length = em_strlen(buffer);
  ret->buffer = em_strdup(buffer);
  return ret;
}

em_result
string_copy(string_t * dst, const string_t * src)
{
  em_result errres = EM_RESULT_OK;
  dst->length      = src->length;
  CHKERR(em_allocarray((void **)&(dst->buffer), sizeof(char), src->length + 1));
  memcpy(dst->buffer, src->buffer, src->length * sizeof(char));
  dst->buffer[dst->length] = '\0';
err:
  return errres;
}

size_t
string_hash(const string_t * self)
{
  return self->buffer[0];
  // Light implementation above.
  size_t ret = 0;
  for(int i = 0; i < self->length; ++i)
    ret ^= self->buffer[i];
  return ret;
}
