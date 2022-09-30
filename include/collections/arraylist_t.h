/** -------------------------------------------
 * @file   arraylist_t.h
 * @brief  Expandable Array (Similar to vector<T> in C++, ArrayList in C#/Java)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/30
 ------------------------------------------- */
#pragma once

#include "emmem.h"
#include <stdlib.h>

// ! Expandable Array (Similar to vector<T> in C++, ArrayList in C#/Java
/* !
 * assertion: length <= capacity.
 */
typedef struct arraylist_t {
  // ! Buffer
  void * buffer;
  // ! Count of containing items.
  size_t length;
  // ! Actual length of buffer.(Capacity)
  size_t capacity;
} arraylist_t;

// ! Zeroing arraylist_t
void arraylist_default(arraylist_t * out);

// ! Append an item to the array list.
/* !
 * /param self The array list to add to.
 * /param item_size sizeof(value).
 * /param value The item to be added.
 * /return The status code. May be OUT_OF_MEMORY, or OUT_OF_INDEX(only Debug build).
 */
em_result arraylist_append(arraylist_t * self, size_t item_size, void * value);

// ! Insert an item to the array list.
/* !
 * /param self The array list to add to.
 * /param index Where to insert.
 * /param item_size sizeof(value).
 * /param value The item to be added.
 * /return The status code. May be OUT_OF_MEMORY, or OUT_OF_INDEX(only Debug build).
 */
em_result arraylist_insert(arraylist_t * self, size_t index, size_t item_size, void * value);

// ! Remove an item from the array list.
/* !
 * /param self The array list to add to.
 * /param out Where to copy the removed item to. (Nullable)
 * /param index Where to remove.
 * /param item_size sizeof(*out).
 */
void arraylist_remove(arraylist_t * self, void * out, size_t index, size_t item_size);

// ! Remove and insert an item from/to the array list.
/* !
 * /param self The array list to add to.
 * /param out Where to copy the removed item to. (Nullable)
 * /param remove_index Where to remove.
 * /param insert_index Where to insert.
 * /param item_size sizeof(*out).
 * /param value The item to be inserted.
 */
void arraylist_removeinsert(arraylist_t * self, void * out, size_t remove_index, size_t insert_index, size_t item_size, void * value);

// ! Get an item from the array list.
/* !
 * /param self The array list to get from.
 * /param out Where to copy the item.
 * /param index Where to get from.
 * /param item_size sizeof(*out).
 */
void arraylist_get(arraylist_t * self, void * out, size_t index, size_t item_size);

// ! Put an item to the array list.(inplace)
/* !
 * /param self The array list to put to.
 * /param index Where to get from.
 * /param item_size sizeof(*value).
 * /param value Where to copy from.
 */
void arraylist_set(arraylist_t * self, size_t index, size_t item_size, void * value);

// ! Freeing the array list.
void arraylist_free(arraylist_t * self);
