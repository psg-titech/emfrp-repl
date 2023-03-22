/** -------------------------------------------
 * @file   machine.c
 * @brief  Emfrp REPL Machine Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/22
 ------------------------------------------- */

#include <stdio.h>
#include "ast.h"
#include "vm/machine.h"
#include "vm/object_t.h"
#include "vm/exec.h"
#include "vm/journal_t.h"
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
  em_result errres = EM_RESULT_OK;
  CHKERR(queue_default(&(out->execution_list)));
  CHKERR(dictionary_new(&(out->nodes)));
  CHKERR(memory_manager_new(&(out->memory_manager)));
  CHKERR(machine_alloc(out, &(out->stack)));
  CHKERR(object_new_stack(out->stack, MACHINE_STACK_SIZE));
  out->variable_table = nullptr;
  CHKERR(machine_new_variable_table(out));
  //return EM_RESULT_OK;
 err: return errres;
}

em_result
machine_exec(machine_t * self, parser_toplevel_t * prog, object_t ** out) {
  em_result errres = EM_RESULT_OK;
  *out = nullptr;
  switch(prog->kind) {
  case PARSER_TOPLEVEL_KIND_EXPR: return exec_ast(self, prog->value.expression, out);
  case PARSER_TOPLEVEL_KIND_DATA: {
    parser_data_t * d = prog->value.data;
    CHKERR(exec_ast(self, d->expression, out));
    if(machine_test_matches(self, &(d->name), *out)) {
      CHKERR(machine_matches(self, &(d->name), *out));
    } else {
      errres = EM_RESULT_INVALID_ARGUMENT;
      goto err;
    }
    break;
  }
  case PARSER_TOPLEVEL_KIND_FUNC: {
    parser_func_t * f = prog->value.func;
    parser_expression_t * e = parser_expression_new_function(f->arguments, f->expression);
    TEST_AND_ERROR(e == nullptr, EM_RESULT_OUT_OF_MEMORY);
    CHKERR(machine_alloc(self, out));
    CHKERR(object_new_function_ast(*out, machine_get_variable_table(self)->this_object_ref, e));
    CHKERR(machine_assign_variable(self, f->name, *out));
    break;
  }
  case PARSER_TOPLEVEL_KIND_NODE: {
    exec_sequence_t * _ = nullptr;
    return machine_add_node_ast(self, &_, prog->value.node);
  }
  case PARSER_TOPLEVEL_KIND_RECORD: {
    object_t * tag = nullptr;
    { // Construct the tag.
      string_t tagname;
      CHKERR(machine_alloc(self, &tag));
      CHKERR(string_copy(&tagname, &(prog->value.record->name)));
      CHKERR(object_new_symbol(tag, tagname));
    }
    // Construct the accessors.
    size_t len = 0;
    for(list_t /*<string_t*>*/ * li = prog->value.record->accessors;
        li != nullptr; li = LIST_NEXT(li), len++) {
      object_t * o = nullptr;
      CHKERR(machine_alloc(self, &o));
      CHKERR(object_new_function_accessor(o, tag, len));
      CHKERR(machine_assign_variable(self, (string_t *)(&(li->value)), o));
    }
    // Construct the constructors.
    CHKERR(machine_alloc(self, out));
    CHKERR(object_new_function_constructor(*out, tag, len));
    CHKERR(machine_assign_variable(self, &(prog->value.record->name), *out));
    break;
  }
  }
  return EM_RESULT_OK;
 err:
  *out = nullptr;
  return errres;
}


em_result
machine_push(machine_t * self, object_t * obj) {
  em_result errres = EM_RESULT_OK;
#if DEBUG
  if(object_kind(self->stack.kind) != EMFRP_OBJECT_STACK) {
    fputs("Illegal stack kind.\n", stdout);
    DEBUGBREAK;
  }
#endif
  object_t * st = self->stack;
  int32_t capacity = 0;
  CHKERR(object_get_int(st->value.stack.capacity, &capacity));
  if(st->value.stack.length >= capacity) {
    CHKERR(em_reallocarray((void **)&(st->value.stack.data),
                           (void *)(st->value.stack.data), capacity + MACHINE_STACK_SIZE, sizeof(object_t *)));
    CHKERR(object_new_int(&(st->value.stack.capacity), capacity + MACHINE_STACK_SIZE));
  }
  st->value.stack.data[st->value.stack.length] = obj;
  st->value.stack.length++;
  // return EM_RESULT_OK;
 err:  return errres;
}

em_result
machine_pop(machine_t * self, object_t ** obj) {
  em_result errres = EM_RESULT_OK;
  object_t * st = self->stack;
#if DEBUG
  if(object_kind(st->kind) != EMFRP_OBJECT_STACK) {
    fputs("Illegal stack kind.\n", stdout);
    DEBUGBREAK;
  }
#endif
  TEST_AND_ERROR(st->value.stack.length == 0, EM_RESULT_STACK_OVERFLOW);
  CHKERR(machine_mark_gray(self, st->value.stack.data[st->value.stack.length - 1]));
  if(obj != nullptr)
    *obj = st->value.stack.data[st->value.stack.length - 1];
  // DO NOT!
  //object_tuple_ith(st, st->value.stack.length - 1) = nullptr;
  st->value.stack.length--;
  //return EM_RESULT_OK;
 err: return errres;
}

bool
machine_match_symbol(object_t * tag, string_t * match_symbol) {
  if(tag == nullptr && match_symbol == nullptr) return true; // BOTH are null.
  return !(tag == nullptr
           || !object_is_pointer(tag)
           || object_kind(tag) != EMFRP_OBJECT_SYMBOL
           || !string_compare(match_symbol, &(tag->value.symbol.value)));
}

em_result
machine_match(machine_t * self, list_t /*<deconstructor_t>*/ * nt, object_t ** vs, int length) {
  em_result errres = EM_RESULT_OK;
  for(int len = 0; nt != nullptr || len != length; ++len, nt = LIST_NEXT(nt)) {
    TEST_AND_ERROR(nt == nullptr || len == length, EM_RESULT_INVALID_ARGUMENT);
    CHKERR(machine_matches(self, (deconstructor_t *)(&(nt->value)), vs[len]));
  }
 err:
  return errres;
}

em_result
machine_matches(machine_t * self, deconstructor_t * deconst, object_t * v) {
  em_result errres = EM_RESULT_OK;
  switch(deconst->kind) {
  case DECONSTRUCTOR_IDENTIFIER: CHKERR(machine_assign_variable(self, deconst->value.identifier, v)); break;
  case DECONSTRUCTOR_ANY: break;
  case DECONSTRUCTOR_TUPLE:
    TEST_AND_ERROR(!object_is_pointer(v) || v == nullptr, EM_RESULT_INVALID_ARGUMENT);
    switch(object_kind(v)) {
    case EMFRP_OBJECT_SYMBOL:
      if(deconst->value.tuple.tag == nullptr
         || deconst->value.tuple.data != nullptr
         || !string_compare(deconst->value.tuple.tag, &(v->value.symbol.value)))
        return EM_RESULT_INVALID_ARGUMENT;
      break;
    case EMFRP_OBJECT_TUPLE1:
      TEST_AND_ERROR(!machine_match_symbol(v->value.tuple1.tag, deconst->value.tuple.tag),
                     EM_RESULT_INVALID_ARGUMENT);
      CHKERR(machine_match(self, deconst->value.tuple.data, &(v->value.tuple1.i0), 1));
      break;
    case EMFRP_OBJECT_TUPLE2:
      TEST_AND_ERROR(!machine_match_symbol(v->value.tuple2.tag, deconst->value.tuple.tag),
                     EM_RESULT_INVALID_ARGUMENT);
      CHKERR(machine_match(self, deconst->value.tuple.data, &(v->value.tuple2.i0), 2));
      break;
    case EMFRP_OBJECT_TUPLEN:
      TEST_AND_ERROR(!machine_match_symbol(v->value.tupleN.tag, deconst->value.tuple.tag),
                     EM_RESULT_INVALID_ARGUMENT);
      CHKERR(machine_match(self, deconst->value.tuple.data, v->value.tupleN.data, v->value.tupleN.length));
      break;
    default: return EM_RESULT_INVALID_ARGUMENT;
    }
    break;
  case DECONSTRUCTOR_INTEGER:
    if(!object_is_integer(v) || deconst->value.integer != object_get_integer(v))
      errres = EM_RESULT_INVALID_ARGUMENT;
    break;
#if EMFRP_ENABLE_FLOATING
  case DECONSTRUCTOR_FLOATING:
#endif
  default: DEBUGBREAK; break;
  }
 err: return errres;
}

bool
machine_test_match(machine_t * self, list_t /*<deconstructor_t>*/ * nt, object_t ** vs, int length) {
  for(int len = 0; nt != nullptr || len != length; ++len, nt = LIST_NEXT(nt))
    if(nt == nullptr || len == length
       || !machine_test_matches(self, (deconstructor_t *)(&(nt->value)), vs[len])) return false;
  return true;
}

bool
machine_test_matches(machine_t * self, deconstructor_t * deconst, object_t * v) {
  switch(deconst->kind) {
  case DECONSTRUCTOR_IDENTIFIER:
  case DECONSTRUCTOR_ANY: return true;
  case DECONSTRUCTOR_TUPLE:
    if(!object_is_pointer(v) || v == nullptr) return false;
    switch(object_kind(v)) {
    case EMFRP_OBJECT_SYMBOL:
      return !(deconst->value.tuple.tag == nullptr
               || deconst->value.tuple.data != nullptr
               || !string_compare(deconst->value.tuple.tag, &(v->value.symbol.value)));
      break;
    case EMFRP_OBJECT_TUPLE1:
      if(!machine_match_symbol(v->value.tuple1.tag, deconst->value.tuple.tag)) return false;
      return machine_test_match(self, deconst->value.tuple.data, &(v->value.tuple1.i0), 1);
    case EMFRP_OBJECT_TUPLE2:
      if(!machine_match_symbol(v->value.tuple2.tag, deconst->value.tuple.tag)) return false;
      return machine_test_match(self, deconst->value.tuple.data, &(v->value.tuple2.i0), 2);
    case EMFRP_OBJECT_TUPLEN:
      if(!machine_match_symbol(v->value.tupleN.tag, deconst->value.tuple.tag)) return false;
      return machine_test_match(self, deconst->value.tuple.data, v->value.tupleN.data, v->value.tupleN.length);
    default: return false;
    }
    break;
  case DECONSTRUCTOR_INTEGER:
    return !(!object_is_integer(v) || deconst->value.integer != object_get_integer(v));
#if EMFRP_ENABLE_FLOATING
  case DECONSTRUCTOR_FLOATING:
#endif
  default: DEBUGBREAK; break;
  }
  return false;
}

bool
string_compare2(void * l, void * r) {  return string_compare(*((string_t **)l), (string_t *)r); }
void
go_check_dependencies(list_t ** dependencies, node_or_tuple_t * nt) {
  switch(nt->kind) {
  case NODE_OR_TUPLE_NONE: return;
  case NODE_OR_TUPLE_NODE: {
    if(nt->value.node == nullptr) return; // itself
    list_t * removed;
    do {
      removed = list_remove(dependencies, string_compare2, &(nt->value.node->name));
      em_free(removed);
    } while(removed != nullptr);
    return;
  }
  case NODE_OR_TUPLE_TUPLE:
    for(int i = 0; i < nt->value.tuple.length; ++i)
      go_check_dependencies(dependencies, &(((node_or_tuple_t *)(nt->value.tuple.buffer))[i]));
    return;
  }
}

em_result
check_dependencies(machine_t * self, parser_expression_t * prog, list_t ** executionlist_head, list_t *** whereto_insert) {
  em_result errres = EM_RESULT_OK;
  list_t * /*<exec_sequence_t *>*/ cur = *executionlist_head;
  list_t * /*<string_t *>*/ dependencies;
  CHKERR(list_default(&dependencies));
  CHKERR(get_dependencies_ast(self, prog, &dependencies));
  // Search where to insert.
  // Inserted after all dependencies are satisfied.
  list_t ** p = executionlist_head;
  for(;!LIST_IS_EMPTY(&dependencies); p = &((*p)->next), cur = LIST_NEXT(cur)) {
    if(LIST_IS_EMPTY(&cur)) { // dependencies are not satisfied.
      list_free(&dependencies);
      return EM_RESULT_MISSING_IDENTIFIER;
    }
    exec_sequence_t * n = (exec_sequence_t *)(&(cur->value));
    if(n->node_definition != nullptr) {
      list_t * removed = (void*)1;
      do {
        removed = list_remove(&dependencies, string_compare2, &(n->node_definition->name));
        em_free(removed);
      } while(removed != nullptr);
    }
    if(n->node_definitions != nullptr)
      go_check_dependencies(&dependencies, n->node_definitions);
  }
  *whereto_insert = p;
  // return EM_RESULT_OK;
 err:
  return errres;
}

bool has_cyclicreference(string_t * newnode_str, list_t * executionlist_head, list_t * cur) {
  // CHECK CYCLIC DEPENDENCIES!
  for(list_t * pcur = executionlist_head; pcur != cur; pcur = LIST_NEXT(pcur)) {
    exec_sequence_t * es = (exec_sequence_t *)(&(pcur->value));
    if(exec_sequence_marked_modified(es)) continue; // Skip!
    if(exec_sequence_program_kind(es) == EMFRP_PROGRAM_KIND_AST
       && check_depends_on_ast(es->program.ast, newnode_str))
      return true;
  }
  return false;
}

bool has_cyclicreference2(deconstructor_t * newnode_str, list_t * executionlist_head, list_t * cur) {
  switch(newnode_str->kind) {
  case DECONSTRUCTOR_IDENTIFIER:
    return has_cyclicreference(newnode_str->value.identifier, executionlist_head, cur);
  case DECONSTRUCTOR_TUPLE:
    for(list_t * li = newnode_str->value.tuple.data; li != nullptr; li = LIST_NEXT(li))
      if(has_cyclicreference2((deconstructor_t *)(&(li->value)), executionlist_head, cur)) return true;
    return false;
  default: DEBUGBREAK; return false;
  }
}

em_result
machine_add_node(machine_t * self, string_t str, node_t ** node_ptr) {
  em_result errres = EM_RESULT_OK;
  if(dictionary_get(&(self->nodes), (void**)node_ptr,(size_t(*)(void *)) string_hash, node_compare, &str))
    return EM_RESULT_OK; // Do nothing.
  node_t new_node = { 0 };
  CHKERR(node_new(&new_node, str));
  CHKERR(dictionary_add2(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, nullptr, nullptr, (void **)node_ptr));
  //return EM_RESULT_OK;
 err:
  return errres;
}

void
machine_cleanup(queue_t /*<exec_sequence_t>*/ * execSeq) {
  for(list_t ** cur = &(execSeq->head); *cur != nullptr; ) {
    exec_sequence_t * es = (exec_sequence_t *)&((*cur)->value);
    if(!exec_sequence_marked_modified(es)) goto next; // Not modified, skip.
    exec_sequence_unmark_modified(es);
    if(!exec_sequence_compact(es)) goto next;// Do not remove this list item.
    list_t * ne = LIST_NEXT(*cur);
    if(ne == nullptr)
      execSeq->last = cur;
    em_free(*cur);
    *cur = ne;
    exec_sequence_free(es);
    continue;
  next: cur = &(LIST_NEXT(*cur));
  }
}

em_result
machine_remove_previous_definition2(machine_t * self, journal_t ** out, string_t * str) {
  node_t * node_ptr;
  if(str == nullptr) return EM_RESULT_INVALID_ARGUMENT;
  if(!dictionary_get(&(self->nodes), (void**)&node_ptr, (size_t(*)(void *))string_hash, node_compare, str)) // Not Found.
    return EM_RESULT_OK;
  return remove_defined_node(self->execution_list.head, out, node_ptr);
}

em_result
machine_remove_previous_definition(machine_t * self, journal_t ** out, deconstructor_t * dt) {
  em_result errres = EM_RESULT_OK;
  switch(dt->kind) {
  case DECONSTRUCTOR_IDENTIFIER:
    return machine_remove_previous_definition2(self, out, dt->value.identifier);
  case DECONSTRUCTOR_TUPLE:
  for(list_t * li = dt->value.tuple.data; li != nullptr; li = LIST_NEXT(li)) {
    deconstructor_t * d = (deconstructor_t *)(&(li->value));
    CHKERR(machine_remove_previous_definition(self, out, d));
  }
  break;
  default: break;
  }
 err:
  return errres;
}

em_result
machine_add_nodes(machine_t * self, deconstructor_t * dt, node_or_tuple_t * node_ptr) {
  em_result errres = EM_RESULT_OK;
  switch(dt->kind) {
  case DECONSTRUCTOR_IDENTIFIER: // string -> node
    CHKERR(machine_add_node(self, *(dt->value.identifier), &(node_ptr->value.node)));
    node_ptr->kind = NODE_OR_TUPLE_NODE;
    break;
  case DECONSTRUCTOR_TUPLE: { // list -> array
    int cnt = 0;
    for(list_t * li = dt->value.tuple.data; li != nullptr; li = LIST_NEXT(li)) cnt++;
    node_ptr->kind = NODE_OR_TUPLE_TUPLE;
    CHKERR(arraylist_new(&(node_ptr->value.tuple), sizeof(node_or_tuple_t), cnt));
    list_t * li = dt->value.tuple.data;
    for(int i = 0; i < cnt; ++i, li = LIST_NEXT(li))
      CHKERR(machine_add_nodes(self, (deconstructor_t *)(&(li->value)), &(((node_or_tuple_t *)(node_ptr->value.tuple.buffer))[i])));
    break;
  }
  default: DEBUGBREAK; break;
  }
 err:
  return errres;
}

em_result
machine_add_node_ast(machine_t * self, exec_sequence_t ** out, parser_node_t * n) {
  em_result errres = EM_RESULT_OK;
  list_t ** /*<exec_sequence_t>*/ whereto_insert;
  exec_sequence_t new_exec_seq = {0};
  exec_sequence_t * new_entry;
  journal_t * journal = nullptr;
  // Remove the previous definition.
  if(n->as != nullptr)
    CHKERR(machine_remove_previous_definition2(self, &journal, n->as));
  CHKERR(machine_remove_previous_definition(self, &journal, &(n->name)));
  // Allocate the new exec_sequence.
  CHKERR(exec_sequence_new_mono_ast(&new_exec_seq, n->expression, nullptr));
  // Dependency Check
  CHKERR(check_dependencies(self, n->expression, &(self->execution_list.head), &whereto_insert));
  // If it contains already-defined nodes, Test the dependency and Try topological sort.
  if(journal != nullptr
     && has_cyclicreference2(&(n->name), self->execution_list.head, *whereto_insert)
     && (n->as == nullptr || has_cyclicreference(n->as, self->execution_list.head, *whereto_insert))) {
    if(topological_sort(self, &(self->execution_list.head), &(self->execution_list.last)) != EM_RESULT_OK) {
      errres = EM_RESULT_CYCLIC_REFERENCE;
      goto err;
    }
  }
  // Adding the new exec_sequence.
  CHKERR(list_add4(whereto_insert, exec_sequence_t, &new_exec_seq, (void**)&new_entry));
  // Update the last node.
  if(LIST_IS_EMPTY(&((*whereto_insert)->next)))
    self->execution_list.last = &((*whereto_insert)->next);
  switch(n->name.kind) {
  case DECONSTRUCTOR_IDENTIFIER: // Single name.
    CHKERR2(err2, machine_add_node(self, *(n->name.value.identifier), &(new_entry->node_definition)));
    break;
  case DECONSTRUCTOR_TUPLE:
    CHKERR2(err2, em_malloc((void**)(&(new_entry->node_definitions)), sizeof(node_or_tuple_t)));
    CHKERR2(err2, machine_add_nodes(self, &(n->name), new_entry->node_definitions));
    if(n->as != nullptr)
      CHKERR2(err2, machine_add_node(self, *(n->as), &(new_entry->node_definition)));
    break;
  default: DEBUGBREAK; break;
  }
  // Clean up the journal.
  if(journal != nullptr) {
    machine_cleanup(&(self->execution_list));
    journal_free(&journal);
    journal = nullptr;
  }
  if(n->init_expression != nullptr) {
    object_t * obj = nullptr;
    em_result res = exec_ast(self, n->init_expression, &obj);
    if(res) {
      // TODO: last failure.
    }
    CHKERR(exec_sequence_update_value_given_object(self, new_entry, obj));
  }
  if(out != nullptr) *out = new_entry;
  return EM_RESULT_OK;
 err2:
  // TODO: Out of Memory failure.
 err:
  revert_from_journal(journal);
  journal_free(&journal);
  return errres;
}

em_result
machine_add_node_callback(machine_t * self, string_t str, exec_callback_t callback) {
  em_result errres = EM_RESULT_OK;
  exec_sequence_t new_exec_seq;
  node_t * node_ptr;
  if(!dictionary_get(&(self->nodes), (void**)&node_ptr, (size_t(*)(void *))string_hash, node_compare, &str)) { // If not already defined.
    CHKERR(machine_add_node(self, str, &node_ptr));
  } else {
    journal_t * journal = nullptr;
    CHKERR(remove_defined_node(self->execution_list.head, &journal, node_ptr));
    machine_cleanup(&(self->execution_list));
    journal_free(&journal);
  }
  CHKERR(exec_sequence_new_mono_callback(&new_exec_seq, callback, node_ptr));
  // CHKERR(callback == nullptr
  //        ? exec_sequence_new_mono_nothing(&new_exec_seq, node_ptr)
  //        : exec_sequence_new_mono_callback(&new_exec_seq, callback, node_ptr));
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
  em_result errres = EM_RESULT_OK;
  
  for(list_t * /*<exec_sequence_t>*/ cur = self->execution_list.head;
      !LIST_IS_EMPTY(&cur); cur = LIST_NEXT(cur))
    CHKERR(exec_sequence_update_last(self, (exec_sequence_t *)(&(cur->value))));
  
  for(list_t * /*<exec_sequence_t>*/ cur = self->execution_list.head;
      !LIST_IS_EMPTY(&cur); cur = LIST_NEXT(cur)) {
    em_result result = exec_sequence_update_value(self, (exec_sequence_t *)(&(cur->value)));
    // TODO: result
  }
  // return EM_RESULT_OK;
 err:
  return errres;
}

em_result
machine_set_value_of_node(machine_t * self, string_t * name, object_t * val) {
  node_t * o = nullptr;
  em_result errres = EM_RESULT_OK;
  if (!dictionary_get(&(self->nodes), (void **)&o, (size_t(*)(void *))string_hash, node_compare, name))
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
  if(dictionary_get(&(self->nodes), (void**)(&ptrToNode), (size_t(*)(void *))string_hash, node_compare, &name)) {
    ptrToNode->action = callback;
    string_free(&name);
  } else {
    node_t new_node = { 0 };
    node_new(&new_node, name);
    new_node.action = callback;
    CHKERR(dictionary_add(&(self->nodes), &new_node, sizeof(node_t), node_hasher, node_compare2, nullptr, nullptr));
  }
  //return EM_RESULT_OK;
err:
  return errres;
}

void
machine_debug_print_definitions(machine_t * self) {
  fputs("=== EXECUTION LIST ===\n", stdout);
  for(list_t * /*<exec_sequence_t>*/ cur = self->execution_list.head;
      cur != nullptr; cur = LIST_NEXT(cur)) {
    exec_sequence_t * n = (exec_sequence_t *)(&(cur->value));
    if(n->node_definitions == nullptr) {
      if(n->node_definition == nullptr)
        fputs("Node<INVALID!>\n", stdout);
      else {
        fputs("Node<", stdout);
        fputs(n->node_definition->name.buffer, stdout);
        fputs(">\n", stdout);
      }
    } else {
      fputs("Node<", stdout);
      node_or_tuple_debug_print(n->node_definitions);
      if(n->node_definition == nullptr)
        fputs(">\n", stdout);
      else {
        fputs("as ", stdout);
        fputs( n->node_definition->name.buffer, stdout);
        fputs(">\n", stdout);
      }
    }
  }
  fputs("======================\n", stdout);
}
