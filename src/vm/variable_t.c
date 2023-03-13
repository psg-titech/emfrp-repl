/** -------------------------------------------
 * @file   variable_t.c
 * @brief  The variable table.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/1
 ------------------------------------------- */
#include "vm/variable_t.h"
#include "string.h"
#include "vm/gc.h"
#include "vm/machine.h"

size_t var_hasher(void * val) { return string_hash(&(((variable_t *)val)->name)); }
// node_t and string_t
bool var_compare(void * l, void * r) { return string_compare(&(((variable_t *)l)->name), (string_t *)r); }
bool var_compare2(void * l, void * r) {
  variable_t * lhs = (variable_t *)l;
  variable_t * rhs = (variable_t *)r;
  return string_compare(&(lhs->name), &(rhs->name));
}

em_result
variable_table_new(struct machine_t * m, variable_table_t * out, variable_table_t * parent) {
  em_result errres = EM_RESULT_OK;
  object_t * o_ref = nullptr;
  CHKERR(machine_alloc(m, &o_ref));
  CHKERR(object_new_variable_table(o_ref, out));
  out->this_object_ref = o_ref;
  out->parent = parent;
  CHKERR(dictionary_new(&(out->table)));
  return EM_RESULT_OK;
 err:
  if(o_ref != nullptr)
    machine_return(m, o_ref);
  return errres;
}


em_result
variable_table_assign(machine_t * m, variable_table_t * self, string_t * name, object_t * value) {
  em_result errres;
  variable_t * var_ptr;
  variable_t new_var = { 0 };
  if(dictionary_get(&(self->table), (void**)&var_ptr, (size_t(*)(void *))string_hash, var_compare, name)) {
    CHKERR(machine_mark_gray(m, var_ptr->value));
    var_ptr->value = value;
    return EM_RESULT_OK;
  }
  CHKERR(string_copy(&(new_var.name), name));
  new_var.value = value;
  CHKERR2(err2, dictionary_add(&(self->table), &new_var, sizeof(variable_t), var_hasher, var_compare2, nullptr, nullptr));
  return EM_RESULT_OK;
 err2: string_free(&(new_var.name));
 err: return errres;
}

bool
variable_table_lookup(variable_table_t * self, object_t ** out, string_t * name) {
  variable_t * var_ptr;
  while(self != nullptr) {
    if(dictionary_get(&(self->table), (void**)&var_ptr, (size_t(*)(void *))string_hash, var_compare, name)) {
      *out = var_ptr->value;
      return true;
    }
    self = self->parent;
  }
  return false;
}

void
variable_deep_free(variable_t * v) {
  string_free(&(v->name));
}

void
variable_table_free(variable_table_t * v) {
  list_t * li;
  FOREACH_DICTIONARY(li, &(v->table)) {
    while(li != nullptr) {
      list_t * ne = LIST_NEXT(li);
      variable_deep_free((variable_t *)(&(li->value)));
      em_free(li);
      li = ne;
    }
  }
  em_free(v);
}
