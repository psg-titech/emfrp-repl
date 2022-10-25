/** -------------------------------------------
 * @file   object_t.c
 * @brief  Emfrp REPL object structure.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/25
 ------------------------------------------- */
#include "vm/object_t.h"
#include <stdio.h>

// ! True Object
const object_t object_true;
// ! False Object
const object_t object_false;

void
object_print(object_t * v) {
  if(object_is_integer(v))
    printf("%d", object_get_integer(v));
  else if(v == &object_true)
    printf("true");
  else if(v == &object_false)
    printf("false");
  else {
    switch(v->kind) {
    case EMFRP_OBJECT_STRING: printf("\"%s\"",v->value.string.value.buffer); break;
    }
  }
}
