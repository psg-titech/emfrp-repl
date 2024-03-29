/** -------------------------------------------
 * @file   dictionary_t.h
 * @brief  Dictionary
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/11
 ------------------------------------------- */
#pragma once
#include "collections/list_t.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
#define DICTIONARY_TABLE_SIZE 16

  // ! Dictionary
  typedef struct dictionary_t
  {
    list_t * values[DICTIONARY_TABLE_SIZE];
  } dictionary_t;

  // ! Construct dictionary_t.
  /* !
 * \param out The result
 * \return The status code
 */
  em_result dictionary_new(dictionary_t * out);

  // ! Adding an item to the dictionary.
  /* !
 * \param out The dictionary to added to.
 * \param value The value to be added. (And passed to comparer as the 2nd argument.)
 * \param value_size sizeof(value)
 * \param hasher The hashing function of value.
 * \param comparer The compare function of value.
 * \param replacer The replace function for existing item. the 1st argument will be the entry to the existing item.(nullable)
 * \param replacer_arg the value which passed to replacer as the 2nd argument.
 * \return The status code
 */
  em_result dictionary_add(
    dictionary_t * out, void * value, size_t value_size, size_t(hasher(void *)),
    bool(comparer(void *, void *)), void(replacer(void *, void *)), void * replacer_arg);

  // ! Adding an item to the dictionary. This returns the pointer of copied buffer.
  /* !
 * \param out The dictionary to added to.
 * \param value The value to be added. (And passed to comparer as the 2nd argument.)
 * \param value_size sizeof(value)
 * \param hasher The hashing function of value.
 * \param comparer The compare function of value.
 * \param replacer The replace function for existing item. the 1st argument will be the entry to the existing item.(nullable)
 * \param replacer_arg the value which passed to replacer as the 2nd argument.
 * \param entry_ptr The pointer of copied buffer.
 * \return The status code
 */
  em_result dictionary_add2(
    dictionary_t * out, void * value, size_t value_size, size_t(hasher(void *)),
    bool(comparer(void *, void *)), void(replacer(void *, void *)), void * replacer_arg,
    void ** entry_ptr);

  // ! Getting an item from the dictionary
  /* !
 * \param self The dictionary to be searched
 * \param out The result
 * \param hasher The hashing function of value.
 * \param comparer The compare function of value.
 * \param search_value It is passed to comparer as the 2nd argument.
 * \return Whether found or not
 */
  bool dictionary_get(
    dictionary_t * self, void ** out, size_t(hasher(void *)), bool(comparer(void *, void *)),
    void * search_value);

  // ! Test an item from the dictionary
  /* !
 * \param self The dictionary to be searched
 * \param hasher The hashing function of value.
 * \param comparer The compare function of value.
 * \param search_value It is passed to comparer as the 2nd argument.
 * \return Whether found or not
 */
  bool dictionary_contains(
    dictionary_t * self, size_t(hasher(void *)), bool(comparer(void *, void *)),
    void *         search_value);

#define FOREACH_DICTIONARY(li, dic)                                                                \
  li = (dic)->values[0];                                                                           \
  for(int i = 0; i < DICTIONARY_TABLE_SIZE; ++i, li = (dic)->values[i])

#ifdef __cplusplus
}
#endif /* __cplusplus */
