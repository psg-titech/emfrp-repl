/** -------------------------------------------
 * @file   string_t.c
 * @brief  Pascal String Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
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
  if(str->buffer != nullptr) free(str->buffer);
  str->buffer = nullptr;
  str->length = 0;
}

void
string_new(string_t * out, char * buffer, const size_t length) {
  out->buffer = buffer;
  out->length = length;
}

string_t *
string_malloc_new(const char * buffer) {
  string_t * ret = (string_t *)em_malloc(sizeof(string_t));
  ret->length = strlen(buffer);
  ret->buffer = em_strdup(buffer);
  return ret;
}
