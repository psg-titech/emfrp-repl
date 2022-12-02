
/** -------------------------------------------
 * @file   exec_sequence_t.c
 * @brief  Execution Sequence.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/2
 ------------------------------------------- */
#include "vm/exec_sequence_t.h"
#include "vm/exec.h"
#include "vm/gc.h"

em_result
exec_sequence_update_value(machine_t * machine, exec_sequence_t * self) {
  em_result errres;
  object_t * new_obj = nullptr;
  switch(self->program_kind) {
  case EXEC_SEQUENCE_PROGRAM_KIND_AST:
    CHKERR(exec_ast(machine, self->program.ast, &new_obj));
    break;
  case EXEC_SEQUENCE_PROGRAM_KIND_CALLBACK:
    new_obj = self->program.callback();
    break;
  case EXEC_SEQUENCE_PROGRAM_KIND_NOTHING: return EM_RESULT_OK;
  }
  if(self->node_definition != nullptr) {
    CHKERR(machine_mark_gray(machine, self->node_definition->value));
    self->node_definition->value = new_obj;
    if(self->node_definition->action != nullptr)
      self->node_definition->action(new_obj);
  }
  errres = EM_RESULT_OK;
 err:
  return errres;
}

em_result
update_node_last(struct machine_t * machine, node_t * n) {
  em_result errres;
  if(n == nullptr) return EM_RESULT_OK;
  CHKERR(machine_mark_gray(machine, n->last));
  n->last = n->value;
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
exec_sequence_update_last(struct machine_t * machine, exec_sequence_t * self) {
  em_result errres;
  CHKERR(update_node_last(machine, self->node_definition));
  return EM_RESULT_OK;
 err:
  return errres;
}
