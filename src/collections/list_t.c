/** -------------------------------------------
 * @file   list_t.c
 * @brief  List
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/14
 ------------------------------------------- */
#include "collections/list_t.h"

em_result
list_add(list_t ** out, size_t value_size, void * value)
{
  list_t *  ret = nullptr;
  em_result err = em_malloc((void **)&ret, sizeof(list_t *) + value_size);
  if(err != EM_RESULT_OK) return err;
  ret->next = *out;
  memcpy(&(ret->value), value, value_size);
  *out = ret;
  return EM_RESULT_OK;
}

em_result
list_add3(list_t ** out, size_t value_size, void * value, void ** entry_ptr)
{
  list_t *  ret = nullptr;
  em_result err = em_malloc((void **)&ret, sizeof(list_t *) + value_size);
  if(err != EM_RESULT_OK) return err;
  ret->next = *out;
  memcpy(&(ret->value), value, value_size);
  *out       = ret;
  *entry_ptr = &(ret->value);
  return EM_RESULT_OK;
}

bool
list_search(list_t * self, void ** out, bool(comparer(void *, void *)), void * search_value)
{
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
list_contains(list_t * self, bool(comparer(void *, void *)), void * search_value)
{
  list_t * cur = self;
  while(cur != nullptr) {
    if(comparer(&(cur->value), search_value)) return true;
    cur = cur->next;
  }
  return false;
}

void
list_free(list_t ** self)
{
  list_t * cur = *self;
  while(cur != nullptr) {
    list_t * f = cur;
    cur        = cur->next;
    em_free(f);
  }
}

list_t *
list_remove(list_t ** self, bool(comparer(void *, void *)), void * search_value)
{
  list_t *  cur      = *self;
  list_t ** delayptr = self;
  while(cur != nullptr) {
    if(comparer(&(cur->value), search_value)) {
      *delayptr = cur->next;
      cur->next = nullptr;
      return cur;
    }
    delayptr = &(cur->next);
    cur      = cur->next;
  }
  return nullptr;
}

list_t *
queue_remove(queue_t * self, bool(comparer(void *, void *)), void * search_value)
{
  list_t *  cur      = self->head;
  list_t ** delayptr = &(self->head);
  while(cur != nullptr) {
    if(comparer(&(cur->value), search_value)) {
      if(cur->next == nullptr) self->last = delayptr;
      *delayptr = cur->next;
      cur->next = nullptr;
      return cur;
    }
    delayptr = &(cur->next);
    cur      = cur->next;
  }
  return nullptr;
}

em_result
queue_enqueue(queue_t * out, size_t value_size, void * value)
{
  list_t *  ret = nullptr;
  em_result err = em_malloc((void **)&ret, sizeof(list_t *) + value_size);
  if(err != EM_RESULT_OK) return err;
  ret->next = nullptr;
  memcpy(&(ret->value), value, value_size);
  *(out->last) = ret;
  out->last    = &(ret->next);
  return EM_RESULT_OK;
}

em_result
queue_enqueue3(queue_t * out, size_t value_size, void * value, void ** entry_ptr)
{
  list_t *  ret = nullptr;
  em_result err = em_malloc((void **)&ret, sizeof(list_t *) + value_size);
  if(err != EM_RESULT_OK) return err;
  ret->next = nullptr;
  memcpy(&(ret->value), value, value_size);
  *(out->last) = ret;
  *entry_ptr   = &(ret->value);
  out->last    = &(ret->next);
  return EM_RESULT_OK;
}

em_result
queue_dequeue(queue_t * self, size_t value_size, void * out)
{
  list_t * v = self->head;
  self->head = v->next;
  if(v->next == nullptr)  // If self.length will be 0.
    self->last = &(v->next);
  memcpy(out, &(v->value), value_size);
  em_free(v);
  return EM_RESULT_OK;
}

em_result
queue_add_head(queue_t * out, size_t value_size, void * value)
{
  em_result errres;
  CHKERR(list_add(&(out->head), value_size, value));
  if(out->last == &(out->head)) out->last = &(out->head->next);
  return EM_RESULT_OK;
err:
  return errres;
}
