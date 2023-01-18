/** -------------------------------------------
 * @file   object_t.c
 * @brief  Emfrp REPL object structure.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/9
 ------------------------------------------- */
#include "vm/object_t.h"
#include <stdio.h>

// ! True Object
object_t object_true;
// ! False Object
object_t object_false;

void
object_print(object_t * v) {
  if (v == nullptr)
    printf("NIL");
  else if(object_is_integer(v))
    printf("%d", object_get_integer(v));
  else if(v == &object_true)
    printf("true");
  else if(v == &object_false)
    printf("false");
  else if(object_is_pointer(v)) {
    switch(object_kind(v)) {
    case EMFRP_OBJECT_SYMBOL: printf("%s", v->value.symbol.value.buffer); break;
    case EMFRP_OBJECT_STRING: printf("\"%s\"",v->value.string.value.buffer); break;
    case EMFRP_OBJECT_TUPLE1:
      if(v->value.tuple1.tag != nullptr)
	object_print(v->value.tuple1.tag);
      printf("(");
      object_print(v->value.tuple1.i0);
      printf(")");
      break;
    case EMFRP_OBJECT_TUPLE2:
      if(v->value.tuple2.tag != nullptr)
	object_print(v->value.tuple2.tag);
      printf("(");
      object_print(v->value.tuple2.i0);
      printf(", ");
      object_print(v->value.tuple2.i1);
      printf(")");
      break;
    case EMFRP_OBJECT_TUPLEN: {
      if(v->value.tupleN.tag != nullptr)
	object_print(v->value.tupleN.tag);
      printf("(");
      object_print(object_tuple_ith(v, 0));
      for(int i = 1; i < v->value.tupleN.length; ++i) {
	printf(", ");
	object_print(object_tuple_ith(v, i));
      }
      printf(")");
      break;
    }
    case EMFRP_OBJECT_FUNCTION: {
      printf("<function object>");
      break;
    }
    case EMFRP_OBJECT_VARIABLE_TABLE: {
      printf("<variable table>");
      break;
    }
    default: break;
    }
  }
}
