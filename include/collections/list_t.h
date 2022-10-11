/** -------------------------------------------
 * @file   list_t.h
 * @brief  List
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/11
 ------------------------------------------- */
#pragma once
#include "emmem.h"
#include "em_result.h"
#include <stdlib.h>
#include <stdbool.h>

// ! Linked List(Stack)
typedef struct list_t {
  // ! Next
  struct list_t * next;
  // ! Value(phantom object.)
  void * value;
} list_t;

// ! Queue
typedef struct queue_t {
  // Head
  struct list_t * head;
  // A pointer to the last list_t::value.
  struct list_t ** last;
} queue_t;

// ! Default constructor of list_t.
/* !
 * /param out The result
 * /return The status code
 */
static inline em_result list_default(list_t ** out) {
  *out = nullptr;
  return EM_RESULT_OK;
}

// ! Adding an item to the list.
/* !
 * /param out The list to add to
 * /param value_size sizeof(value)
 * /param value The value to be added
 * /return The status code
 */
em_result list_add(list_t ** out, size_t value_size, void * value);
#define list_add2(out, ty, val) list_add(out, sizeof(ty), val)

// ! Adding an item to the list.
/* !
 * /param out The list to add to
 * /param value_size sizeof(value)
 * /param value The value to be added
 * /param entry_ptr The pointer of copied buffer
 * /return The status code
 */
em_result list_add3(list_t ** out, size_t value_size, void * value, void ** entry_ptr);


// ! Search an item.
/* !
 * /param self The list
 * /param out The result
 * /param comparer The compare function
 * /param search_value Passed to comparer as the 2nd argument.
 * /return Whether found or not
 */
bool list_search(list_t * self, void ** out, bool(comparer(void *, void *)), void * search_value);

// ! Test an item.
/* !
 * /param self The list
 * /param comparer The compare function
 * /param search_value Passed to comparer as the 2nd argument.
 * /return Whether found or not
 */
bool list_contains(list_t * self, bool(comparer(void *, void *)), void * search_value);

#define LIST_NEXT(li) (li)->next
#define LIST_IS_EMPTY(li) (*(li) == nullptr)

// ! Freeing the list.
/* !
 * /param self The list to be freed.
 */
void list_free(list_t ** self);

// ! Remove an item from the list.
/* !
 * /param self The list to remove from.
 * /param comparer The compare function.
 * /param search_value An item to be removed(be passed to comparer as the 2nd argument).
 * /return The pointer to the removed list_t. If it is nullptr, there are no matched items.
 */
list_t * list_remove(list_t ** self, bool(comparer(void *, void *)), void * search_value);

// ! Remove an item from the list.
/* !
 * /param self The list to remove from.
 * /param comparer The compare function.
 * /param search_value An item to be removed(be passed to comparer as the 2nd argument).
 * /param undo Where the return value removed from.
 * /return The pointer to the removed list_t. If it is nullptr, there are no matched items.
 */
list_t * list_remove2(list_t ** self, bool(comparer(void *, void *)), void * search_value, list_t *** undo);

// ! Default counstroctor of queue_t.
/* !
 * /param out The result
 * /return The status code
 */
static inline em_result queue_default(queue_t * out) {
  em_result errres;
  CHKERR(list_default(&(out->head)));
  out->last = &(out->head);
  return EM_RESULT_OK;
 err: return errres;
}

// ! Enqueue
/* !
 * /param out The queue to add to
 * /param value_size sizeof(value)
 * /param value The value to be added
 * /return The status code
 */
em_result queue_enqueue(queue_t * out, size_t value_size, void * value);
#define queue_enqueue2(out, ty, val) queue_enqueue(out, sizeof(ty), val)

// ! Dequeue
/* !
 * /param self The queue to dequeue from
 * /param value_size sizeof(value)
 * /param out The result
 * /return The status code
 */
em_result queue_dequeue(queue_t * self, size_t value_size, void * out);
#define queue_dequeue2(self, ty, out) queue_dequeue(self, sizeof(ty), out)
