/** -------------------------------------------
 * @file   arraylist_t.c
 * @brief  Expandable Array (Similar to vector<T> in C++, ArrayList in C#/Java)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/10
 ------------------------------------------- */
#include "collections/arraylist_t.h"

void
arraylist_default(arraylist_t * out) {
  out->buffer = nullptr;
  out->length = 0;
  out->capacity = 0;
}

em_result
arraylist_resize(arraylist_t * self, size_t item_size) {
  em_result errres = EM_RESULT_OK;
  int new_len = self->length + 16;
  void * new_buf = nullptr;
  CHKERR(em_reallocarray(&new_buf, self->buffer, new_len, item_size));
  if(new_buf == nullptr)
    return EM_RESULT_OUT_OF_MEMORY;
  self->buffer = new_buf;
  self->capacity = new_len;
  //return EM_RESULT_OK;
 err:
  return errres;
}

em_result
arraylist_append(arraylist_t * self, size_t item_size, void * value) {
  item_size /= sizeof(char);
  em_result errres;
  if(self->length + 1 > self->capacity)
    CHKERR(arraylist_resize(self, item_size));
  memcpy(self->buffer + (item_size * self->length), value, item_size);
  self->length++;
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
arraylist_insert(arraylist_t * self, size_t index, size_t item_size, void * value) {
  item_size /= sizeof(char);
  em_result errres;
#if DEBUG
  if(index > self->length || index < 0)
    return EM_RESULT_OUT_OF_INDEX;
  if(index == self->length)
    return arraylist_append(self, item_size, value);
#endif
  if(self->length + 1 > self->capacity)
    CHKERR(arraylist_resize(self, item_size));
  memmove(self->buffer + (item_size * (index + 1)), self->buffer + (item_size * index), (self->length - index) * item_size); 
  memcpy(self->buffer + (item_size * index), value, item_size);
  self->length++;
  return EM_RESULT_OK;
 err:
  return errres;
}

void
arraylist_remove(arraylist_t * self, void * out, size_t index, size_t item_size) {
  item_size /= sizeof(char);
  if(item_size > 0 && out != nullptr)
    memcpy(out, self->buffer + (item_size * index), item_size);
  memcpy(self->buffer + (item_size * index),
	 self->buffer + (item_size * index) + item_size,
	 item_size * (self->length - index - 1));
  self->length--;
}

void arraylist_removeinsert(arraylist_t * self, void ** out, size_t remove_index, size_t insert_index, size_t item_size, void * value) {
  item_size /= sizeof(char);
  if(item_size > 0 && out != nullptr)
    memcpy(out, self->buffer + (item_size * remove_index), item_size);
  if(remove_index == insert_index) {
  } else if(remove_index < insert_index)
    memcpy(self->buffer + (item_size * remove_index),
	   self->buffer + (item_size * remove_index) + item_size,
	   item_size * (insert_index - remove_index));
  else
    memmove(self->buffer + (item_size * (insert_index + 1)),
            self->buffer + (item_size * insert_index),
            (remove_index - insert_index) * item_size);
  
  memcpy(self->buffer + (item_size * insert_index), value, item_size);
}

void
arraylist_get(arraylist_t * self, void * out, size_t index, size_t item_size) {
#if DEBUG
  if(self->length <= index || index < 0) {
    fprintf(stderr, "Out of Index(arraylist).");
    DEBUGBREAK;
  }
#endif
  memcpy(out, self->buffer + (item_size * index), item_size);
}

void
arraylist_set(arraylist_t * self, size_t index, size_t item_size, void * value) {
#if DEBUG
  if(self->length <= index || index < 0) {
    fprintf(stderr, "Out of Index(arraylist).");
    DEBUGBREAK;
  }
#endif
  memcpy(self->buffer + (item_size * index), value, item_size);
}

void
arraylist_free(arraylist_t * self) {
  em_free(self->buffer);
  self->buffer = nullptr;
  self->length = 0;
}
