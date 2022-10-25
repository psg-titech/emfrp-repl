/** -------------------------------------------
 * @file   machine.c
 * @brief  Emfrp REPL Machine Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/25
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
  out->executing_node_name = nullptr;
  return EM_RESULT_OK;
 err: return errres;
}

bool
string_compare2(void * l, void * r) {  return string_compare(*((string_t **)l), (string_t *)r); }
void
node_cleaner(void * v) { node_deep_free((node_t *)v);}

em_result
check_dependencies(parser_expression_t * prog, list_t ** executionlist_head, list_t *** whereto_insert) {
  em_result errres;
  list_t * /*<node_t *>*/ cur = *executionlist_head;
  list_t ** p = executionlist_head;
  list_t * /*<string_t *>*/ dependencies;
  CHKERR(list_default(&dependencies));
  CHKERR(get_dependencies_ast(prog, &dependencies));
  // Search where to insert.
  // Inserted after all dependencies are satisfied.
  while(!LIST_IS_EMPTY(&dependencies)) {
    list_t * removed;
    if(LIST_IS_EMPTY(&cur)) { // dependencies are not satisfied.
      list_free(&dependencies);
      return EM_RESULT_MISSING_IDENTIFIER;
    }
    node_t * n = (node_t *)(cur->value);
    while((removed = list_remove(&dependencies, string_compare2, &(n->name))) != nullptr)
      em_free(removed);
    p = &((*p)->next);
    cur = LIST_NEXT(cur);
  }
  *whereto_insert = p;
  return EM_RESULT_OK;
 err:
  return errres;
}

bool has_cyclicreference(string_t * newnode_str, parser_expression_t * prog, list_t * executionlist_head, list_t * whereto_insert) {
  // CHECK CYCLIC DEPENDENCIES!
  for(list_t * pcur = executionlist_head; pcur != whereto_insert; pcur = pcur->next) {
    if(((node_t *)(pcur->value))->program_kind == NODE_PROGRAM_KIND_AST &&
       check_depends_on_ast(((node_t *)(pcur->value))->program.ast, newnode_str))
      return true;
  }
  return false;
}

em_result
machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog, parser_expression_t * initialization) {
  em_result errres;
  node_t new_node = { 0 };
  node_t * ptr_to_new_node;
  list_t ** whereto_insert;
  CHKERR(node_new_ast(&new_node, str, prog));
  if (initialization != nullptr) {
    self->executing_node_name = &(ptr_to_new_node->name);
    CHKERR(exec_ast(self, initialization, &(new_node.value)));
    self->executing_node_name = nullptr;
  }
  node_t * out_val = nullptr;
  bool isDefined = dictionary_get(&(self->nodes), (void**)&out_val, string_hasher, node_compare, &str);
  if(out_val != nullptr)
    new_node.action = out_val->action;

  if(isDefined) {
    em_result reason;
    list_t * /*<node_t *> */ revert = list_remove(&(self->execution_list.head), string_compare2, &str);
    if (&revert->next == self->execution_list.last)
        self->execution_list.last = &self->execution_list.head;
    em_free(revert);
    reason = check_dependencies(prog, &self->execution_list.head, &whereto_insert);
    if(reason != EM_RESULT_OK)
      goto err2;
    if(has_cyclicreference(&str, prog, self->execution_list.head, *whereto_insert)) {
      reason = EM_RESULT_CYCLIC_REFERENCE;
      goto err2;
    }
    CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, node_cleaner, (void**)&ptr_to_new_node));
    CHKERR(list_add2(whereto_insert, node_t *, &ptr_to_new_node));
    if(LIST_IS_EMPTY(&((*whereto_insert)->next)))
      self->execution_list.last = &((*whereto_insert)->next);
    goto end;
err2: // REVERTING
    if (out_val->program_kind == NODE_PROGRAM_KIND_AST) {
      CHKERR(check_dependencies(out_val->program.ast, &self->execution_list.head, &whereto_insert));
      // Cyclic Reference Checking is skipped.
    }
    else
      whereto_insert = &(self->execution_list.head);
    CHKERR(list_add2(whereto_insert, node_t *, &out_val));
    if (LIST_IS_EMPTY(&((*whereto_insert)->next))) 
        self->execution_list.last = &((*whereto_insert)->next);
    errres = reason;
    goto err;
  }  else {
    CHKERR(check_dependencies(prog, &(self->execution_list.head), &whereto_insert));
    CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, node_cleaner, (void**)&ptr_to_new_node));
    CHKERR(queue_enqueue2(&(self->execution_list), node_t *, &ptr_to_new_node));
  }
  end:
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
  CHKERR(queue_add_head2(&(self->execution_list), node_t *, &ptr_to_node));
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

em_result
machine_indicate(machine_t * self, string_t * names, int count_names) {
  // names is currently ignored. i.e. All of nodes are executed.
  em_result errres;
  list_t * /*<node_t*>*/ cur = self->execution_list.head;
  while(!LIST_IS_EMPTY(&cur)) {
    node_t * v = (node_t *)cur->value;
    self->executing_node_name = &(v->name);
    if (v->program_kind == NODE_PROGRAM_KIND_AST) {
      CHKERR(exec_ast(self, v->program.ast, &(v->value)));
    }
    else if (v->program_kind == NODE_PROGRAM_KIND_CALLBACK)
      v->value = v->program.callback();
    if (v->action != nullptr)
      v->action(v->value);
    cur = LIST_NEXT(cur);
  }
  self->executing_node_name = nullptr;
  return EM_RESULT_OK;
 err:
  self->executing_node_name = nullptr;
  return errres;
}

em_result
machine_set_value_of_node(machine_t * self, string_t * name, object_t * val) {
  node_t * o = nullptr;
  if (!dictionary_get(&(self->nodes), (void **)&o, string_hasher, node_compare, name))
      return EM_RESULT_MISSING_IDENTIFIER;
  o->value = val;
  return EM_RESULT_OK;
}

em_result
machine_add_output_node(machine_t * self, string_t name, node_event_delegate_t callback) {
  em_result errres;
  node_t * ptrToNode = nullptr;
  if(dictionary_get(&(self->nodes), (void**)(&ptrToNode), string_hasher, node_compare, &name)) {
    ptrToNode->action = callback;
    string_free(&name);
  } else {
    node_t new_node = { 0 };
    node_t * ptr_to_node = nullptr;
    node_new_nothing(&new_node, name);
    new_node.action = callback;
    CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, node_cleaner, (void **)&ptr_to_node));
    CHKERR(queue_add_head2(&self->execution_list, node_t *, &ptr_to_node));
  }
  return EM_RESULT_OK;
err:
  return errres;
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
