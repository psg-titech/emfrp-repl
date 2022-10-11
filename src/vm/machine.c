/** -------------------------------------------
 * @file   machine.c
 * @brief  Emfrp REPL Machine Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/11
 ------------------------------------------- */

#include "vm/machine.h"
#include "vm/exec.h"
#include <stdio.h>

size_t string_hasher(void * s) { return string_hash((string_t *)s); }
size_t node_hasher(void * val) { return string_hash(&(((node_t *)val)->name)); }
// node_t and string_t
bool node_compare(void * l, void * r) { return string_compare(&(((node_t *)l)->name), (string_t *)r); }

bool
node_compare2(void * l, void * r) {
  node_t * lhs = (node_t *)l;
  node_t * rhs = (node_t *)r;
  return string_compare(&(lhs->name), &(rhs->name));
}

em_result
machine_new(machine_t * out) {
  em_result errres;
  CHKERR(queue_default(&(out->execution_list)));
  CHKERR(dictionary_new(&(out->nodes)));
  return EM_RESULT_OK;
 err: return errres;
}

bool
string_compare2(void * l, void * r) {  return string_compare(*((string_t **)l), (string_t *)r); }
void
node_cleaner(void * v) { node_deep_free((node_t *)v);}

em_result
machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog) {
  em_result errres;
  node_t new_node = { 0 };
  node_t * ptr_to_new_node;
  CHKERR(node_new_ast(&new_node, str, prog));
  CHKERR(exec_ast(self, prog, &new_node.value));
  bool isDefined = dictionary_contains(&(self->nodes), string_hasher, node_compare, &str);
  CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, node_cleaner, (void**)&ptr_to_new_node));
  if(isDefined) {
    list_t * /*<string_t *>*/ dependencies;
    list_t * /*<node_t *>*/ cur = self->execution_list.head;
    list_t ** /*<node_t *>*/ delay = &self->execution_list.head;
    list_t * /*<string_t *>*/ removed = nullptr;
    CHKERR(list_default(&dependencies));
    CHKERR(get_dependencies_ast(prog, &dependencies));
    while(!LIST_IS_EMPTY(&cur)) {
      node_t * n = (node_t *)(cur->value);
      do {
        removed = list_remove(&dependencies, string_compare2, &(n->name));
        if(removed != nullptr) em_free(removed);
      } while(removed != nullptr);
      delay = &((*delay)->next);
      cur = LIST_NEXT(cur);
      if(LIST_IS_EMPTY(&dependencies)) break;  // All dependencies are satisfied.
    }
    // Also all dependencies are not satisfied.
    if(!LIST_IS_EMPTY(&dependencies)) {
      errres = EM_RESULT_MISSING_IDENTIFIER;
      list_free(&dependencies);
      goto err;
    }
    // CHECK CYCLIC DEPENDENCIES!
    list_t * pcur = self->execution_list.head;
    while(pcur != cur) {
      if(check_depends_on_ast(((node_t *)(pcur->value))->program.ast, &str)) {
        errres = EM_RESULT_CYCLIC_REFERENCE;
        goto err;
      }
      pcur = pcur->next;
    }
    /*
    if(string_compare(&(((node_t *)(&(cur->value)))->name), &str)) {
      cur = cur->next;
      if(cur == nullptr) {
	printf("BUG!\n");
      }
    }
    */
    /* list_t <node_t *> * */ removed = list_remove(&(self->execution_list.head), string_compare2, &str);
    em_free(removed);
    CHKERR(list_add2(delay, node_t *, &ptr_to_new_node));
  }
  else
    CHKERR(queue_enqueue2(&(self->execution_list), node_t *, &ptr_to_new_node));
  return EM_RESULT_OK;
 err:
  if(new_node.name.buffer != nullptr)
    string_free(&(new_node.name));
  object_free(new_node.value);
  return errres;
}

em_result
machine_add_node_callback(machine_t * self, string_t str, node_callback_t callback) {
  em_result errres;
  node_t new_node = { 0 };
  node_t * ptr_to_node = nullptr;
  if(callback == nullptr){
    CHKERR(node_new_nothing(&new_node, str));
    new_node.value = nullptr;
  } else {
    CHKERR(node_new_callback(&new_node, str, callback));
    new_node.value = callback();
  }
  bool isDefined = dictionary_contains(&(self->nodes), string_hasher, node_compare, &str);
  CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, node_cleaner, (void**)&ptr_to_node));
  if(isDefined) {
    list_remove(&(self->execution_list.head), string_compare2, &str);
  }
  CHKERR(list_add2(&self->execution_list.head, node_t *, &ptr_to_node));
  return EM_RESULT_OK;
 err:
  if(new_node.name.buffer != nullptr)
    string_free(&(new_node.name));
  em_free(new_node.value);
  return errres;
}

bool
machine_search_node(machine_t * self, object_t ** out, string_t * name) {
  node_t * o;
  if(!dictionary_get(&(self->nodes), (void**)&o, node_hasher, node_compare, name))
    return false;
  *out = o->value;
  return true;
}

bool
macihne_is_defined(machine_t * self, string_t * name) {
  return dictionary_contains(&(self->nodes), node_hasher, node_compare, name);
}

void
machine_debug_print_definitions(machine_t * self) {
  list_t * cur = self->execution_list.head;
  while(cur != nullptr) {
    node_t * n = (node_t *)(cur->value);
    printf("Node<%s>\n", n->name.buffer);
    cur = cur->next;
  }
}
