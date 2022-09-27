/** -------------------------------------------
 * @file   dictionary_t.h
 * @brief  Dictionary
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/27
 ------------------------------------------- */
#pragma once
#include "collections/list_t.h"
#include <stdbool.h>
#define DICTIONARY_TABLE_SIZE 16

// ! Dictionary
typedef struct dictionary_t {
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
 * \param value The value to be added.
 * \param value_size sizeof(value)
 * \param hasher The hashing function of value.
 * \return The status code
 */
em_result dictionary_add(dictionary_t * out, void * value, size_t value_size, size_t(hasher(void *)));

// ! Getting an item from the dictionary
/* !
 * \param self The dictionary to be searched
 * \param out The result
 * \param hasher The hashing function of value.
 * \param comparer The compare function of value.
 * \param search_value It is passed to comparer as the 2nd argument.
 * \return Whether found or not
 */
bool dictionary_get(dictionary_t * self, void ** out, size_t(hasher(void *)), bool(comparer(void *, void *)), void * search_value);

// ! Test an item from the dictionary
/* !
 * \param self The dictionary to be searched
 * \param hasher The hashing function of value.
 * \param comparer The compare function of value.
 * \param search_value It is passed to comparer as the 2nd argument.
 * \return Whether found or not
 */
bool dictionary_contains(dictionary_t * self, size_t(hasher(void *)), bool(comparer(void *, void *)), void * search_value);
