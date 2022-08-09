#include "string_t.h"
#include <stdlib.h>
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
string_new(string_t * out, char * buffer, size_t length) {
  out->buffer = buffer;
  out->length = length;
}
