/** -------------------------------------------
 * @file   machine.c
 * @brief  Emfrp REPL Machine Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/27
 ------------------------------------------- */

#include "vm/machine.h"
#include "vm/exec.h"
#include <stdio.h>

size_t
value_of_node_hasher(void * val) {
  value_of_node_t * value = (value_of_node_t *)val;
  return string_hash(&(value->name));
}

bool
value_of_node_compare(void * l, void * r) {
  value_of_node_t * lhs = (value_of_node_t *)l;
  string_t * rhs = (string_t *)r;
  return string_compare(&(lhs->name), rhs);
}

em_result
machine_new(machine_t * out) {
  em_result errres;
  CHKERR(queue_default(&(out->nodes)));
  CHKERR(dictionary_new(&(out->node_values)));
  return EM_RESULT_OK;
 err: return errres;
}

em_result
machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog) {
  em_result errres;
  node_t new_node;
  value_of_node_t new_val;
  if(dictionary_contains(&(self->node_values), value_of_node_hasher, value_of_node_compare, &str)) {
    printf("Error: Currently re-definition is not supported.\n");
    return EM_RESULT_UNKNOWN_ERR;
  }
  CHKERR(string_copy(&(new_val.name), &str));
  CHKERR(exec_ast(self, prog, &new_val.value));
  CHKERR(node_new_ast(&new_node, str, prog));
  CHKERR(queue_enqueue2(&(self->nodes), node_t, &new_node));
  CHKERR(dictionary_add(&(self->node_values), &new_val, sizeof(value_of_node_t), value_of_node_hasher));
  list_t * hoge = nullptr;
  for(int i = 0; i< 16; ++i) {
    if(self->node_values.values[i] != nullptr) {
      hoge = self->node_values.values[i];
      break;
    }
  }
 err: return errres;
}

bool
machine_search_node(machine_t * self, object_t ** out, string_t * name) {
  value_of_node_t * o;
  if(!dictionary_get(&(self->node_values), (void**)&o, value_of_node_hasher, value_of_node_compare, name))
    return false;
  *out = o->value;
  return true;
}

bool
macihne_is_defined(machine_t * self, string_t * name) {
  return dictionary_contains(&(self->node_values), value_of_node_hasher, value_of_node_compare, name);
}
