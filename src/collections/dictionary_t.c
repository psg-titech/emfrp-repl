/** -------------------------------------------
 * @file   dictionary_t.c
 * @brief  Dictionary
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/11
 ------------------------------------------- */
#include "collections/dictionary_t.h"

em_result
dictionary_new(dictionary_t * out) {
  em_result errres;
  for(int i = 0; i < DICTIONARY_TABLE_SIZE; ++i)
    CHKERR(list_default(&(out->values[i])));
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
dictionary_add(dictionary_t * out, void * value, size_t value_size, size_t(hasher(void *)), bool(comparer(void *, void *)), void(replacer(void *))) {
  em_result errres;
  size_t hashed = hasher(value) % DICTIONARY_TABLE_SIZE;
  void * overwrite;
  if(list_search(out->values[hashed], &overwrite, comparer, value)) {
    if(replacer != nullptr)
      replacer(overwrite);
    memcpy(overwrite, value, value_size);
  }else
    CHKERR(list_add(&(out->values[hashed]), value_size, value));
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
dictionary_add2(dictionary_t * out, void * value, size_t value_size, size_t(hasher(void *)), bool(comparer(void *, void *)), void(replacer(void *)), void ** entry_ptr) {
  em_result errres;
  size_t hashed = hasher(value) % DICTIONARY_TABLE_SIZE;
  void * overwrite;
  if(list_search(out->values[hashed], &overwrite, comparer, value)) {
    if(replacer != nullptr)
      replacer(overwrite);
    *entry_ptr = overwrite;
    memcpy(overwrite, value, value_size);
  }else
    CHKERR(list_add3(&(out->values[hashed]), value_size, value, entry_ptr));
  return EM_RESULT_OK;
 err:
  return errres;
}

bool dictionary_get(dictionary_t * self, void ** out,  size_t(hasher(void *)), bool(comparer(void *, void *)), void * search_value) {
  em_result errres;
  size_t hashed = hasher(search_value) % DICTIONARY_TABLE_SIZE;
  return list_search(self->values[hashed], out, comparer, search_value);
}

bool dictionary_contains(dictionary_t * self, size_t(hasher(void *)), bool(comparer(void *, void *)), void * search_value) {
  em_result errres;
  size_t hashed = hasher(search_value) % DICTIONARY_TABLE_SIZE;
  return list_contains(self->values[hashed], comparer, search_value);
}
