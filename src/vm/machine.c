/** -------------------------------------------
 * @file   machine.c
 * @brief  Emfrp REPL Machine Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/2
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
  CHKERR(memory_manager_new(&(out->memory_manager)));
  return EM_RESULT_OK;
 err: return errres;
}

bool
string_compare2(void * l, void * r) {  return string_compare(*((string_t **)l), (string_t *)r); }
void
node_cleaner(void * v) { node_deep_free((node_t *)v);}
bool
exec_sequence_node_ptr_compare(void * l/*pointer of exec_sequence_t*/, void * r /*pointer of node_t * */) {
  exec_sequence_t * exec_seq = (exec_sequence_t *)l;
  node_t ** n = (node_t **)r;
  if(exec_seq->node_definition == *n) return true;
  return false;
}

em_result
check_dependencies(parser_expression_t * prog, list_t ** executionlist_head, list_t *** whereto_insert) {
  em_result errres;
  list_t * /*<exec_sequence_t *>*/ cur = *executionlist_head;
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
    exec_sequence_t * n = (exec_sequence_t *)(&(cur->value));
    while((removed = list_remove(&dependencies, string_compare2, &(n->node_definition->name))) != nullptr)
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
    exec_sequence_t * es = (exec_sequence_t *)(&(pcur->value));
    if(es->program_kind == EXEC_SEQUENCE_PROGRAM_KIND_AST && check_depends_on_ast(es->program.ast, newnode_str))
      return true;
  }
  return false;
}

em_result
machine_add_node(machine_t * self, string_t str, node_t ** node_ptr) {
  em_result errres;
  //  if(dictionary_get(&(self->nodes), (void**)&node_ptr, string_hasher, node_compare, &str))
  //    return EM_RESULT_OK; // Do nothing.
  node_t new_node = { 0 };
  CHKERR(node_new(&new_node, str));
  CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, node_cleaner, (void **)node_ptr));
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog, parser_expression_t * initialization) {
  em_result errres;
  list_t ** /*<exec_sequence_t *>*/ whereto_insert;
  node_t * node_ptr;
  exec_sequence_t new_exec_seq = {0};
  if(!dictionary_get(&(self->nodes), (void**)&node_ptr, string_hasher, node_compare, &str)) { // If not already defined.
    CHKERR(check_dependencies(prog, &(self->execution_list.head), &whereto_insert));
    CHKERR(machine_add_node(self, str, &node_ptr));
    printf("OK!\n");
    CHKERR(exec_sequence_new_mono_ast(&new_exec_seq, prog, node_ptr));
    CHKERR(queue_enqueue2(&(self->execution_list), exec_sequence_t, &new_exec_seq));
  } else {  
    em_result reason;
    list_t * /*<exec_sequence_t *> */ revert = queue_remove(&(self->execution_list), exec_sequence_node_ptr_compare, &node_ptr);
    exec_sequence_t * es = (exec_sequence_t *)(&(revert->value));
    reason = check_dependencies(prog, &self->execution_list.head, &whereto_insert);
    if(reason != EM_RESULT_OK)
      goto err2;
    if(has_cyclicreference(&str, prog, self->execution_list.head, *whereto_insert)) {
      reason = EM_RESULT_CYCLIC_REFERENCE;
      goto err2;
    }
    CHKERR(exec_sequence_new_mono_ast(&new_exec_seq, prog, node_ptr));
    CHKERR(list_add2(whereto_insert, exec_sequence_t, &new_exec_seq));
    if(LIST_IS_EMPTY(&((*whereto_insert)->next)))
      self->execution_list.last = &((*whereto_insert)->next);
    goto end;
err2: // REVERTING
    
    if (es->program_kind == EXEC_SEQUENCE_PROGRAM_KIND_AST) {
      CHKERR(check_dependencies(es->program.ast, &self->execution_list.head, &whereto_insert));
      // Cyclic Reference Checking is skipped.
    }
    else
      whereto_insert = &(self->execution_list.head);
    CHKERR(list_add2(whereto_insert, exec_sequence_t, es));
    em_free(revert);
    if (LIST_IS_EMPTY(&((*whereto_insert)->next))) 
        self->execution_list.last = &((*whereto_insert)->next);
    errres = reason;
    goto err;
  }
  end:
  if (initialization != nullptr)
    CHKERR(exec_ast(self, initialization, &(node_ptr->value)));
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
machine_add_node_callback(machine_t * self, string_t str, exec_callback_t callback) {
  em_result errres;
  exec_sequence_t new_exec_seq;
  node_t * node_ptr;
  if(!dictionary_get(&(self->nodes), (void**)&node_ptr, string_hasher, node_compare, &str)) { // If not already defined.
    CHKERR(machine_add_node(self, str, &node_ptr));
  } else {  
    em_result reason;
    list_t * /*<exec_sequence_t *> */ revert = queue_remove(&(self->execution_list), exec_sequence_node_ptr_compare, &node_ptr);
    em_free(revert);
  }
  if(callback == nullptr) {
    CHKERR(exec_sequence_new_mono_nothing(&new_exec_seq, node_ptr));
  } else {
    CHKERR(exec_sequence_new_mono_callback(&new_exec_seq, callback, node_ptr));
  }
  CHKERR(list_add2(&(self->execution_list.head), exec_sequence_t, &new_exec_seq));
  if(LIST_IS_EMPTY(&(self->execution_list.head->next)))
    self->execution_list.last = &(self->execution_list.head->next);
  
 err:
  return errres;
}

bool
machine_lookup_node(machine_t * self, node_t ** out, string_t * name) {
  return dictionary_get(&(self->nodes), (void**)out, node_hasher, node_compare, name);
}

bool
macihne_is_defined(machine_t * self, string_t * name) {
  return dictionary_contains(&(self->nodes), node_hasher, node_compare, name);
}

em_result
machine_indicate(machine_t * self, string_t * names, int count_names) {
  // names is currently ignored. i.e. All of nodes are executed.
  em_result errres;
  list_t * /*<exec_sequence_t>*/ cur;
  cur = self->execution_list.head;
  while(!LIST_IS_EMPTY(&cur)) {
    exec_sequence_t * es = (exec_sequence_t *)(&(cur->value));
    CHKERR(exec_sequence_update_last(self, es));
    cur = LIST_NEXT(cur);
  }
  cur = self->execution_list.head;
  while(!LIST_IS_EMPTY(&cur)) {
    exec_sequence_t * es = (exec_sequence_t *)(&(cur->value));
    em_result result = exec_sequence_update_value(self, es);
    cur = LIST_NEXT(cur);
  }
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
machine_set_value_of_node(machine_t * self, string_t * name, object_t * val) {
  node_t * o = nullptr;
  em_result errres;
  if (!dictionary_get(&(self->nodes), (void **)&o, string_hasher, node_compare, name))
      return EM_RESULT_MISSING_IDENTIFIER;
  CHKERR(machine_mark_gray(self, o->last));
  o->last = o->value;
  o->value = val;
  return EM_RESULT_OK;
 err:
  return errres;
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
    void * _ = nullptr;
    node_new(&new_node, name);
    new_node.action = callback;
    CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, node_cleaner, &_));
  }
  return EM_RESULT_OK;
err:
  return errres;
}

void
machine_debug_print_definitions(machine_t * self) {
  list_t * cur = self->execution_list.head;
  while(cur != nullptr) {
    exec_sequence_t * n = (exec_sequence_t *)(&(cur->value));
    printf("Node<%s>\n", n->node_definition->name.buffer);
    cur = LIST_NEXT(cur);
  }
}
