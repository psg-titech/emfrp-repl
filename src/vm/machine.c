/** -------------------------------------------
 * @file   machine.c
 * @brief  Emfrp REPL Machine Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/13
 ------------------------------------------- */

#include "vm/machine.h"
#include "vm/exec.h"
#include <stdio.h>
#include "vm/journal_t.h"
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
void
go_check_dependencies(list_t ** dependencies, node_or_tuple_t nt) {
  switch(nt.kind) {
  NODE_OR_TUPLE_NONE: return;
  NODE_OR_TUPLE_NODE: {
      list_t * removed;
      string_t * n = &(nt.value.node->name);
      while((removed = list_remove(dependencies, string_compare2, n)) != nullptr)
	em_free(removed);
      return;
    }
  NODE_OR_TUPLE_TUPLE:
    for(int i = 0; i < nt.value.tuple.length; ++i)
      go_check_dependencies(dependencies, ((node_or_tuple_t *)(nt.value.tuple.buffer))[i]);
    return;
  }
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
    if(n->node_definition != nullptr) {
      while((removed = list_remove(&dependencies, string_compare2, &(n->node_definition->name))) != nullptr)
	em_free(removed);
    }
    if(n->node_definitions != nullptr)
      go_check_dependencies(&dependencies, *(n->node_definitions));
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

void
machine_cleanup(queue_t /*<exec_sequence_t>*/ * execSeq) {
  list_t ** cur = &(execSeq->head);
  while(*cur != nullptr) {
    exec_sequence_t * es = (exec_sequence_t *)&((*cur)->value);
    if(!exec_sequence_marked_modified(es)) // Not modified, skip.
      goto next;
    exec_sequence_unmark_modified(es);
    if(!exec_sequence_compact(es)) // Do not remove this list item.
      goto next;
    list_t * ne = (*cur)->next;
    if(ne == nullptr)
      execSeq->last = cur;
    em_free(*cur);
    *cur = ne;
    exec_sequence_free(es);
    continue;
  next: cur = &((*cur)->next);
  }
}

em_result
machine_add_node_ast(machine_t * self, string_t str, parser_expression_t * prog, parser_expression_t * initialization) {
  em_result errres = EM_RESULT_OK;
  list_t ** /*<exec_sequence_t *>*/ whereto_insert;
  node_t * node_ptr;
  exec_sequence_t new_exec_seq = {0};
  if(!dictionary_get(&(self->nodes), (void**)&node_ptr, string_hasher, node_compare, &str)) { // If not already defined.
    CHKERR(check_dependencies(prog, &(self->execution_list.head), &whereto_insert));
    CHKERR(machine_add_node(self, str, &node_ptr));
    CHKERR(exec_sequence_new_mono_ast(&new_exec_seq, prog, node_ptr));
    CHKERR(queue_enqueue2(&(self->execution_list), exec_sequence_t, &new_exec_seq));
  } else {  
    em_result reason;
    journal_t * journal = nullptr;
    CHKERR(remove_defined_node(self->execution_list.head, &journal, node_ptr));
    reason = check_dependencies(prog, &self->execution_list.head, &whereto_insert);
    if(reason != EM_RESULT_OK)
      goto err2;
    if(has_cyclicreference(&str, prog, self->execution_list.head, *whereto_insert)) {
      reason = EM_RESULT_CYCLIC_REFERENCE;
      // TODO Full Topological Sort.
      goto err2;
    }
    CHKERR(exec_sequence_new_mono_ast(&new_exec_seq, prog, node_ptr));
    CHKERR(list_add2(whereto_insert, exec_sequence_t, &new_exec_seq));
    if(LIST_IS_EMPTY(&((*whereto_insert)->next)))
      self->execution_list.last = &((*whereto_insert)->next);
    machine_cleanup(&(self->execution_list));
    journal_free(&journal);
    goto end;
err2: // REVERTING
    revert_from_journal(journal);
    errres = reason;
    goto err;
  }
  end:
  if (initialization != nullptr) {
    object_t * new_obj = nullptr;
    CHKERR(exec_ast(self, initialization, &new_obj));
    CHKERR(exec_sequence_update_value_given_object(self, &new_exec_seq, new_obj));
  }
  // return EM_RESULT_OK;
 err:
  return errres;
}

em_result
machine_add_node_callback(machine_t * self, string_t str, exec_callback_t callback) {
  em_result errres = EM_RESULT_OK;
  exec_sequence_t new_exec_seq;
  node_t * node_ptr;
  if(!dictionary_get(&(self->nodes), (void**)&node_ptr, string_hasher, node_compare, &str)) { // If not already defined.
    CHKERR(machine_add_node(self, str, &node_ptr));
  } else {
    journal_t * journal = nullptr;
    CHKERR(remove_defined_node(self->execution_list.head, &journal, node_ptr));
    machine_cleanup(&(self->execution_list));
    journal_free(&journal);
  }
  if(callback == nullptr) {
    CHKERR(exec_sequence_new_mono_nothing(&new_exec_seq, node_ptr));
  } else {
    CHKERR(exec_sequence_new_mono_callback(&new_exec_seq, callback, node_ptr));
  }
  CHKERR(list_add2(&(self->execution_list.head), exec_sequence_t, &new_exec_seq));
  if(LIST_IS_EMPTY(&(self->execution_list.head->next)))
    self->execution_list.last = &(self->execution_list.head->next);
  // return EM_RESULT_OK;
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
  list_t * /*<exec_sequence_t>*/ cur = self->execution_list.head;
  for(;!LIST_IS_EMPTY(&cur); cur = LIST_NEXT(cur)) {
    CHKERR(exec_sequence_update_last(self, (exec_sequence_t *)(&(cur->value))));
  }
  for(cur = self->execution_list.head; !LIST_IS_EMPTY(&cur); cur = LIST_NEXT(cur)) {
    exec_sequence_t * es = (exec_sequence_t *)(&(cur->value));
    em_result result = exec_sequence_update_value(self, es);
  }
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
machine_set_value_of_node(machine_t * self, string_t * name, object_t * val) {
  node_t * o = nullptr;
  em_result errres = EM_RESULT_OK;
  if (!dictionary_get(&(self->nodes), (void **)&o, string_hasher, node_compare, name))
      return EM_RESULT_MISSING_IDENTIFIER;
  CHKERR(machine_mark_gray(self, o->last));
  o->last = o->value;
  o->value = val;
  // return EM_RESULT_OK;
 err:
  return errres;
}

em_result
machine_add_output_node(machine_t * self, string_t name, node_event_delegate_t callback) {
  em_result errres = EM_RESULT_OK;
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
  //return EM_RESULT_OK;
err:
  return errres;
}

void
machine_debug_print_definitions(machine_t * self) {
  list_t * cur = self->execution_list.head;
  while(cur != nullptr) {
    exec_sequence_t * n = (exec_sequence_t *)(&(cur->value));
    if(n->node_definitions == nullptr) {
      if(n->node_definition == nullptr)
	printf("Node<INVALID!>\n");
      else
	printf("Node<%s>\n", n->node_definition->name.buffer);
    } else {
      printf("Node<");
      node_or_tuple_debug_print(n->node_definitions);
      if(n->node_definition == nullptr)
	printf(">\n");
      else
	printf("as %s>\n", n->node_definition->name.buffer);
    }
    cur = LIST_NEXT(cur);
  }
}
