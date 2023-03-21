/** -------------------------------------------
 * @file   gc.h
 * @brief  A memory manager(snapshot GC)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/21
 ------------------------------------------- */

#pragma once
#include "em_result.h"
#include "vm/object_t.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
// ! Size of objects.
#define MEMORY_MANAGER_HEAP_SIZE 512
// ! When memory_manager_t::remaining is below this, the gc starts.
#define MEMORY_MANAGER_GC_START_SIZE (MEMORY_MANAGER_HEAP_SIZE / 2)
// ! Size of work list.
#define MEMORY_MANAGER_WORK_LIST_SIZE 256 // = 1KiB

struct machine_t;

// ! The state of memory_manager_t.
typedef enum memory_manager_t_state {
  // ! Idle state
  MEMORY_MANAGER_STATE_IDLE,
  // ! Marking phase
  MEMORY_MANAGER_STATE_MARK,
  // ! Sweep phase
  MEMORY_MANAGER_STATE_SWEEP
} memory_manager_t_state;

// ! The memory manager(snapshot GC)
typedef struct memory_manager_t {
  // ! The state
  memory_manager_t_state state;
  // ! The heap space
  object_t space[MEMORY_MANAGER_HEAP_SIZE];
  // ! List of free cells.
  object_t * freelist;
  // ! size(freelist)
  int remaining;
  // ! Gray-colored list.
  object_t * worklist[MEMORY_MANAGER_WORK_LIST_SIZE];
  // ! worklist.iter
  int worklist_top;
  // ! sweeper for snapshot GC.
  int sweeper;
} memory_manager_t;

// ! Push to work list without checking the state. (Coloring with gray.)
/* !
 * /param self The memory manager
 * /param obj The object to be colored with gray.
 * /return The result
 */
em_result
memory_manager_push_worklist_uncheck_state(memory_manager_t * self, object_t * obj);

// ! Push to work list (Coloring with gray.)
/* !
 * /param self The memory manager
 * /param obj The object to be colored with gray.
 * /return The result
 */
static inline em_result
memory_manager_push_worklist(memory_manager_t * self, object_t * obj) {
  return self->state == MEMORY_MANAGER_STATE_MARK ?
    memory_manager_push_worklist_uncheck_state(self, obj) : EM_RESULT_OK;
}
#define machine_mark_gray(self, obj) memory_manager_push_worklist((self)->memory_manager, obj)

// ! Allocating and newing  memory_manager_t.
/* !
 * /param out The place to be placed.
 * /return The result(may return from em_malloc.)
 */
em_result memory_manager_new(memory_manager_t ** out);

// ! Allocate a cell.
/* !
 * /param self The machine(may start the garbage collection.)
 * /param o The place to be allocated.
 * /return The result(may return out of memory.)
 */
em_result memory_manager_alloc(struct machine_t * self, object_t ** o);
#define machine_alloc memory_manager_alloc

// ! Force to GC(TBD)
em_result memory_manager_force_gc(memory_manager_t * self);
#define machine_force_gc memory_manager_force_gc

// ! Return the object to Memory manager immediately.
/* !
 * /param self The memory manager.
 * /param v The object to be returned.
 */
void memory_manager_return(memory_manager_t * self, object_t * v);
#define machine_return(self, v) memory_manager_return(self->memory_manager, v)

#ifdef __cplusplus
}
#endif /* __cplusplus */
