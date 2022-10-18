/** -------------------------------------------
 * @file   list_t.c
 * @brief  List
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/18
 ------------------------------------------- */
#include "collections/list_t.h"

em_result
list_add(list_t ** out, size_t value_size, void * value) {
  list_t * ret = (list_t *)em_malloc(sizeof(list_t *) + value_size);
  ret->next = *out;
  memcpy(&(ret->value), value, value_size);
  *out = ret;
  return EM_RESULT_OK;
}

em_result
list_add3(list_t ** out, size_t value_size, void * value, void ** entry_ptr) {
  list_t * ret = (list_t *)em_malloc(sizeof(list_t *) + value_size);
  ret->next = *out;
  memcpy(&(ret->value), value, value_size);
  *out = ret;
  *entry_ptr = &(ret->value);
  return EM_RESULT_OK;
}
  
bool
list_search(list_t * self, void ** out, bool(comparer(void *, void *)), void * search_value) {
  list_t * cur = self;
  while(cur != nullptr) {
    if(comparer(&(cur->value), search_value)) {
      *out = &(cur->value);
      return true;
    }
    cur = cur->next;
  }
  return false;
}

bool
list_contains(list_t * self, bool(comparer(void *, void *)), void * search_value) {
  list_t * cur = self;
  while(cur != nullptr) {
    if(comparer(&(cur->value), search_value))
      return true;
    cur = cur->next;
  }
  return false;
}

void
list_free(list_t ** self) {
  list_t * cur = *self;
  while(cur != nullptr) {
    list_t * f = cur;
    cur = cur->next;
    em_free(f);
  }
}

list_t *
list_remove(list_t ** self, bool(comparer(void *, void *)), void * search_value) {
  list_t * cur = *self;
  list_t ** delayptr = self;
  while(cur != nullptr) {
    if(comparer(&(cur->value), search_value)) {
      *delayptr = cur->next;
      cur->next = nullptr;
      return cur;
    }
    delayptr = &(cur->next);
    cur = cur->next;
  }
  return nullptr;
}

list_t *
list_remove2(list_t ** self, bool(comparer(void *, void *)), void * search_value, list_t *** undo) {
  list_t * cur = *self;
  list_t ** delayptr = self;
  while(cur != nullptr) {
    if(comparer(&(cur->value), search_value)) {
      *undo = delayptr;
      *delayptr = cur->next;
      cur->next = nullptr;
      return cur;
    }
    delayptr = &(cur->next);
    cur = cur->next;
  }
  return nullptr;
}

em_result
queue_enqueue(queue_t * out, size_t value_size, void * value) {
  list_t * ret = (list_t *)em_malloc(sizeof(list_t *) + value_size);
  ret->next = nullptr;
  memcpy(&(ret->value), value, value_size);
  *(out->last) = ret;
  out->last = &(ret->next);
  return EM_RESULT_OK;
}

em_result
queue_dequeue(queue_t * self, size_t value_size, void * out) {
  list_t * v = self->head;
  self->head = v->next;
  if(v->next == nullptr) // If self.length will be 0.
    self->last = &(v->next);
  memcpy(out, &(v->value), value_size);
  em_free(v);
  return EM_RESULT_OK;
}

em_result
queue_add_head(queue_t * out, size_t value_size, void * value) {
  em_result errres;
  CHKERR(list_add(&(out->head), value_size, value));
  if (out->last == &(out->head))
    out->last = &(out->head->next);
  return EM_RESULT_OK;
err:
  return errres;
}