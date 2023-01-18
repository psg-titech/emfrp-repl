/** -------------------------------------------
 * @file   gc.c
 * @brief  A memory manager(snapshot GC)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/14
 ------------------------------------------- */
#include "emmem.h"
#include "ast.h"
#include "vm/gc.h"
#include "vm/machine.h"
#include "vm/variable_t.h"

#define MARK_LIMIT 10
#define SWEEP_LIMIT 10

em_result
memory_manager_new(memory_manager_t ** out) {
  em_result errres;
  memory_manager_t * m;
  CHKERR(em_malloc((void **)&m, sizeof(memory_manager_t)));
  *out = m;
  object_t * next = nullptr;
  for(int i = MEMORY_MANAGER_HEAP_SIZE - 1; i >= 0; --i) {
    object_new_freelist(&(m->space[i]), next);
    next = &(m->space[i]);
  }
  m->freelist = m->space;
  m->remaining = MEMORY_MANAGER_HEAP_SIZE;
  m->worklist[0] = nullptr;
  m->worklist_top = 0;
  m->state = MEMORY_MANAGER_STATE_IDLE;
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
push_worklist(memory_manager_t * self, object_t * obj) {
  if(!object_is_pointer(obj)) return EM_RESULT_OK;
  if(obj == nullptr) return EM_RESULT_OK;
  if(object_is_marked(obj)) return EM_RESULT_OK;
  object_mark(obj);
  //printf("marked: %d\n", ((int)obj - (int)self->space) / sizeof(object_t));
  if(MEMORY_MANAGER_WORK_LIST_SIZE == self->worklist_top)
    return EM_RESULT_GC_WORKLIST_OVERFLOW;
  self->worklist[self->worklist_top] = obj;
  self->worklist_top++;
  return EM_RESULT_OK;
}

em_result
memory_manager_push_worklist(memory_manager_t * self, object_t * obj) {
  if(self->state == MEMORY_MANAGER_STATE_MARK)
    return push_worklist(self, obj);
  else
    return EM_RESULT_OK;
}

em_result
memory_manager_mark(memory_manager_t * self, int mark_limit) {
  em_result errres = EM_RESULT_OK;
  for(int i = 0; i < mark_limit; ++i) {
    if(self->worklist_top == 0) break;
    self->worklist_top--;
    object_t * cur = self->worklist[self->worklist_top];
    switch(object_kind(cur)) {
    case EMFRP_OBJECT_TUPLE1:
      CHKERR(push_worklist(self, cur->value.tuple1.i0));
      CHKERR(push_worklist(self, cur->value.tuple1.tag));
      break;
    case EMFRP_OBJECT_TUPLE2:
      CHKERR(push_worklist(self, cur->value.tuple2.i0));
      CHKERR(push_worklist(self, cur->value.tuple2.i1));
      CHKERR(push_worklist(self, cur->value.tuple2.tag));
      break;
    //case EMFRP_OBJECT_STACK:
    case EMFRP_OBJECT_TUPLEN:
      for(size_t i = 0; i < cur->value.tupleN.length; ++i)
	CHKERR(push_worklist(self, object_tuple_ith(cur, i)));
      CHKERR(push_worklist(self, cur->value.tupleN.tag));
      break;
    case EMFRP_OBJECT_VARIABLE_TABLE: if(cur->value.variable_table.ptr != nullptr) {
      list_t * li;
      FOREACH_DICTIONARY(li, &(cur->value.variable_table.ptr->table)) {
	void * v;
        FOREACH_LIST(v, li) {
          variable_t * n = (variable_t *)v;
	  //printf("root: %s %d\n", n->name.buffer , ((int)n->value - (int)self->memory_manager->space) / sizeof(object_t));
          CHKERR(push_worklist(self, n->value));
        }
      }
      if(cur->value.variable_table.ptr->parent != nullptr)
	return push_worklist(self, cur->value.variable_table.ptr->parent->this_object_ref);
      break;
      } break;
    case EMFRP_OBJECT_FUNCTION: {
      switch(cur->value.function.kind) {
      case EMFRP_PROGRAM_KIND_AST:
	return push_worklist(self, cur->value.function.function.ast.closure);
      case EMFRP_PROGRAM_KIND_NOTHING: return EM_RESULT_OK;
      case EMFRP_PROGRAM_KIND_CALLBACK: return EM_RESULT_OK;
      case EMFRP_PROGRAM_KIND_RECORD_CONSTRUCT:
	return push_worklist(self, cur->value.function.function.construct.tag);
      case EMFRP_PROGRAM_KIND_RECORD_ACCESS:
	return push_worklist(self, cur->value.function.function.access.tag);
      default: DEBUGBREAK; break;
      }
      break;
    }
    case EMFRP_OBJECT_FREE:
    case EMFRP_OBJECT_STRING:
    case EMFRP_OBJECT_SYMBOL: break;
    }
  }
 err:
  return errres;
}

void
memory_manager_sweep(memory_manager_t * self, int sweep_limit) {
  for(int i = 0; i < sweep_limit; ++i) {
    if(self->sweeper >= MEMORY_MANAGER_HEAP_SIZE) break;
    object_t * cur = &self->space[self->sweeper];
    if(object_kind(cur) != EMFRP_OBJECT_FREE) {
      if(object_is_marked(cur))
	object_unmark(cur);
      else {
	switch(object_kind(cur)) {
	case EMFRP_OBJECT_SYMBOL: string_free(&(cur->value.symbol.value)); break;
	//case EMFRP_OBJECT_STACK:
	case EMFRP_OBJECT_TUPLEN: em_free(cur->value.tupleN.data); break;
	case EMFRP_OBJECT_VARIABLE_TABLE: variable_table_free(cur->value.variable_table.ptr); break;
	case EMFRP_OBJECT_FUNCTION:
	  switch(cur->value.function.kind) {
	  case EMFRP_PROGRAM_KIND_AST:
	    cur->value.function.function.ast.program->value.function.reference_count--;
	    if(cur->value.function.function.ast.program->value.function.reference_count <= 0)
	      parser_expression_free(cur->value.function.function.ast.program);
	    break;
	  case EMFRP_PROGRAM_KIND_NOTHING: break;
	  case EMFRP_PROGRAM_KIND_CALLBACK: break;
	  case EMFRP_PROGRAM_KIND_RECORD_CONSTRUCT: break;
	  case EMFRP_PROGRAM_KIND_RECORD_ACCESS: break;
	  default: DEBUGBREAK; break;
	  }
	  break;
	default: break;
	}
	object_new_freelist(cur, self->freelist);
	self->freelist = cur;
	//printf("recycled: %d\n", ((int)cur - (int)self->space) / sizeof(object_t));
	self->remaining++;
      }
    }
    self->sweeper++;
  }
}

em_result
memory_manager_gc(struct machine_t * self,
                  int mark_limit, int sweep_limit) {
  em_result errres;
  memory_manager_t * mm = self->memory_manager;
  switch(mm->state) {
  case MEMORY_MANAGER_STATE_IDLE:
    if(mm->remaining <= MEMORY_MANAGER_GC_START_SIZE) {
      mm->worklist_top = 0;
      CHKERR(push_worklist(mm, self->stack));
      CHKERR(push_worklist(mm, machine_get_variable_table(self)->this_object_ref));
      list_t * li;
      FOREACH_DICTIONARY(li, &self->nodes) {
	void * v;
        FOREACH_LIST(v, li) {
          node_t * n = (node_t *)v;
	  //printf("root: %s %d\n", n->name.buffer , ((int)n->value - (int)self->memory_manager->space) / sizeof(object_t));
          CHKERR(push_worklist(mm, n->value));
        }
      }
      mm->state = MEMORY_MANAGER_STATE_MARK;
    }
    break;
  case MEMORY_MANAGER_STATE_MARK:
    CHKERR(memory_manager_mark(mm, mark_limit));
    if(mm->worklist_top == 0) {
      mm->state = MEMORY_MANAGER_STATE_SWEEP;
      mm->sweeper = 0;
    }
    break;
  case MEMORY_MANAGER_STATE_SWEEP:
    memory_manager_sweep(mm, sweep_limit);
    if(mm->sweeper >= MEMORY_MANAGER_HEAP_SIZE)
      mm->state = MEMORY_MANAGER_STATE_IDLE;
    break;
  }
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
memory_manager_alloc(machine_t * self, object_t ** o) {
  em_result errres;
  CHKERR(memory_manager_gc(self, MARK_LIMIT, SWEEP_LIMIT));
  if(self->memory_manager->remaining == 0)
    return EM_RESULT_OUT_OF_MEMORY;
  self->memory_manager->remaining--;
  *o = self->memory_manager->freelist;
  self->memory_manager->freelist = (*o)->value.free.next;
  (*o)->kind = 0;
  if(self->memory_manager->state == MEMORY_MANAGER_STATE_SWEEP &&
     &(self->memory_manager->space[self->memory_manager->sweeper]) <= *o)
    object_mark(*o);
  //printf("allocated: %d\n", ((int)*o - (int)self->memory_manager->space) / sizeof(object_t));
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result memory_manager_force_gc(memory_manager_t * self) {
  DEBUGBREAK;
  return 0;
}

void
memory_manager_return(memory_manager_t * self, object_t * v) {
  if(!object_is_pointer(v)) return;
  v->kind = EMFRP_OBJECT_FREE;
  v->value.free.next = self->freelist;
  self->freelist = v;
}
