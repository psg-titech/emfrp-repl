/** -------------------------------------------
 * @file   exec_sequence_t.c
 * @brief  Execution Sequence.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/1
 ------------------------------------------- */
#include "vm/exec_sequence_t.h"
#include "vm/exec.h"
#include "vm/gc.h"

em_result
exec_sequence_update_value(machine_t * machine, exec_sequence_t * es) {
  em_result errres;
  object_t * new_obj = nullptr;
  switch(es->program_kind) {
  case EXEC_SEQUENCE_PROGRAM_KIND_AST:
    CHKERR(exec_ast(machine, es->program.ast, &new_obj));
    break;
  case EXEC_SEQUENCE_PROGRAM_KIND_CALLBACK:
    new_obj = es->program.callback();
    break;
  case EXEC_SEQUENCE_PROGRAM_KIND_NOTHING: return EM_RESULT_OK;
  }
  if(es->node_definition != nullptr) {
    CHKERR(machine_mark_gray(machine, es->node_definition->value));
    es->node_definition->value = new_obj;
    if(es->node_definition->action != nullptr)
      es->node_definition->action(new_obj);
  }
  errres = EM_RESULT_OK;
 err:
  return errres;
}
