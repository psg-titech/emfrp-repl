/** -------------------------------------------
 * @file   machine.c
 * @brief  Emfrp REPL Machine Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/29
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

bool
value_of_node_compare2(void * l, void * r) {
  value_of_node_t * lhs = (value_of_node_t *)l;
  value_of_node_t * rhs = (value_of_node_t *)r;
  return string_compare(&(lhs->name), &(rhs->name));
}

bool
node_compare(void * l, void * r) {
  node_t * ll = (node_t *)l;
  string_t * rr = (string_t *) r;
  return string_compare(&(ll->name), rr);
}

em_result
machine_new(machine_t * out) {
  em_result errres;
  CHKERR(queue_default(&(out->nodes)));
  CHKERR(dictionary_new(&(out->node_values)));
  return EM_RESULT_OK;
 err: return errres;
}

bool
string_compare2(void * l, void * r) {
  string_t ** ll = (string_t **)l;
  string_t * rr = (string_t *)r;
  return string_compare(*ll, rr);
}

void
node_cleaner(void * v) {
  node_t * n = (node_t *)v;
  string_free(&(n->name));
  if(n->program_kind == NODE_PROGRAM_KIND_AST)
    parser_expression_free(n->program.ast);
}

void
value_of_node_cleaner(void * v) {
  value_of_node_t * vn = (value_of_node_t *)v;
  string_free(&(vn->name));
  object_free(vn->value);
}

em_result
machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog) {
  em_result errres;
  node_t new_node = { 0 };
  value_of_node_t new_val = { 0 };
  CHKERR(string_copy(&(new_val.name), &str));
  CHKERR(exec_ast(self, prog, &new_val.value));
  CHKERR(node_new_ast(&new_node, str, prog));
  if(dictionary_contains(&(self->node_values), value_of_node_hasher, value_of_node_compare, &str)) {
    list_t * /*<string_t *>*/ dependencies;
    list_t * /*<node_t>*/ cur = self->nodes.head;
    list_t ** /*<node_t>*/ delay = &self->nodes.head;
    list_t * /*<string_t *>*/ removed = nullptr;
    CHKERR(list_default(&dependencies));
    CHKERR(get_dependencies_ast(prog, &dependencies));
    while(!LIST_IS_EMPTY(&dependencies) && !LIST_IS_EMPTY(&cur)) { // actually null check for cur is not needed.
      node_t * n = (node_t *)&(cur->value);
      removed = list_remove(&dependencies, string_compare2, &(n->name));
      em_free(removed);
      delay = &((*delay)->next);
      cur = LIST_NEXT(cur);
      if(removed != nullptr && LIST_IS_EMPTY(&dependencies)) break;
    }
    if(!LIST_IS_EMPTY(&dependencies)) {
      errres = EM_RESULT_MISSING_IDENTIFIER;
      list_free(&dependencies);
      goto err;
    }
    // CHECK DEPENDENCIES!
    list_t * pcur = self->nodes.head;
    while(pcur != cur) {
      if(check_depends_on_ast(((node_t * )(&(pcur->value)))->program.ast, &str)) {
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
    /* list_t <node_t> * */ removed = list_remove(&(self->nodes.head), node_compare, &str);

    node_cleaner(&(removed->value));
    em_free(removed);
       
    CHKERR(list_add2(delay, node_t, &new_node));
  }
  else
    CHKERR(queue_enqueue2(&(self->nodes), node_t, &new_node));
  CHKERR(dictionary_add(&(self->node_values), &new_val, sizeof(value_of_node_t), value_of_node_hasher, value_of_node_compare2, value_of_node_cleaner));
  return EM_RESULT_OK;
 err:
  if(new_val.name.buffer != nullptr)
    string_free(&(new_val.name));
  if(new_node.name.buffer != nullptr)
    string_free(&(new_node.name));
  object_free(new_val.value);
  return errres;
}

em_result
machine_add_node_callback(machine_t * self, string_t str, node_callback_t callback) {
  em_result errres;
  node_t new_node = { 0 };
  value_of_node_t new_val = { 0 };
  CHKERR(string_copy(&(new_val.name), &str));
  if(callback == nullptr){
    new_val.value = nullptr;
    CHKERR(node_new_nothing(&new_node, str));
  } else {
    new_val.value = callback();
    CHKERR(node_new_callback(&new_node, str, callback));
  }
  if(dictionary_contains(&(self->node_values), value_of_node_hasher, value_of_node_compare, &str)) {
    list_t * /*<node_t> */ removed = list_remove(&(self->nodes.head), node_compare, &str);
    node_cleaner(&(removed->value));
    em_free(removed);
  }
  CHKERR(list_add2(&self->nodes.head, node_t, &new_node));
  CHKERR(dictionary_add(&(self->node_values), &new_val, sizeof(value_of_node_t), value_of_node_hasher, value_of_node_compare2, value_of_node_cleaner));
  return EM_RESULT_OK;
 err:
  if(new_val.name.buffer != nullptr)
    string_free(&(new_val.name));
  if(new_node.name.buffer != nullptr)
    string_free(&(new_node.name));
  object_free(new_val.value);
  return errres;
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

void
machine_debug_print_definitions(machine_t * self) {
  list_t * cur = self->nodes.head;
  while(cur != nullptr) {
    node_t * n = (node_t *)&(cur->value);
    printf("Node<%s>\n", n->name.buffer);
    cur = cur->next;
  }
}
