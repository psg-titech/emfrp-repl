#pragma once
#include "em_result.h"
#include "misc.h"
#include <string.h>

typedef struct string_t {
  char * buffer;
  size_t length;
} string_t;

void string_null(string_t * str);
void string_free(string_t * str);
void string_new(string_t * out, char * buffer, size_t length);
static inline void string_new1(string_t * out, char * buffer) { string_new(out, buffer, strlen(buffer)); }
