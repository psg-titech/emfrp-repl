/** -------------------------------------------
 * @file   object_t.c
 * @brief  Emfrp REPL object structure.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/11/27
 ------------------------------------------- */
#include "vm/object_t.h"
#include <stdio.h>

// ! True Object
object_t object_true;
// ! False Object
object_t object_false;

void
object_print(object_t * v) {
  if(object_is_integer(v))
    printf("%d", object_get_integer(v));
  else if(v == &object_true)
    printf("true");
  else if(v == &object_false)
    printf("false");
  else {
    switch(object_kind(v)) {
    case EMFRP_OBJECT_STRING: printf("\"%s\"",v->value.string.value.buffer); break;
    case EMFRP_OBJECT_TUPLE1:
      printf("(");
      object_print(v->value.tuple1.i0);
      printf(")");
      break;
    case EMFRP_OBJECT_TUPLE2:
      printf("(");
      object_print(v->value.tuple2.i0);
      printf(", ");
      object_print(v->value.tuple2.i1);
      printf(")");
      break;
    case EMFRP_OBJECT_TUPLEN: {
      printf("(");
      object_print(object_tuple_ith(v, 0));
      for(int i = 1; i < v->value.tupleN.length; ++i) {
	printf(", ");
	object_print(object_tuple_ith(v, i));
      }
      printf(")");
      break;
    }
    }
  }
}
