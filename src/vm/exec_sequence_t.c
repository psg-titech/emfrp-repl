
/** -------------------------------------------
 * @file   exec_sequence_t.c
 * @brief  Execution Sequence.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/6
 ------------------------------------------- */
#include "vm/exec_sequence_t.h"
#include "vm/exec.h"
#include "vm/gc.h"

em_result
exec_sequence_set_nil(machine_t * machine, node_or_tuple_t nt) {
  em_result errres = EM_RESULT_OK;
  switch(nt.kind) {
  case NODE_OR_TUPLE_NONE: return EM_RESULT_OK;
  case NODE_OR_TUPLE_NODE: {
    node_t * n = nt.value.node;
    CHKERR(machine_mark_gray(machine, n->value));
    n->value = nullptr;
    if(n->action != nullptr)
      n->action(nullptr);
    break;
  }
  case NODE_OR_TUPLE_TUPLE: {
    arraylist_t /* <node_or_tuple_t> */ * al = &nt.value.tuple;
    for(int i = 0; i < al->length; ++i)
      CHKERR(exec_sequence_set_nil(machine, ((node_or_tuple_t *)(al->buffer))[i]));
    break;
  }
  }
  return EM_RESULT_OK;
 err:
  return errres;
}

em_result
exec_sequence_set_nodes(machine_t * machine, node_or_tuple_t nt, object_t * v) {
  em_result errres;
  if(v == nullptr) return exec_sequence_set_nil(machine, nt);
  switch(nt.kind) {
  case NODE_OR_TUPLE_NONE: return EM_RESULT_OK;
  case NODE_OR_TUPLE_NODE: {
    node_t * n = nt.value.node;
    if(n == nullptr) return EM_RESULT_OK;
    CHKERR(machine_mark_gray(machine, n->value));
    n->value = v;
    if(n->action != nullptr)
      n->action(v);
    return EM_RESULT_OK;
  }
  case NODE_OR_TUPLE_TUPLE: {
    arraylist_t /* <node_or_tuple_t> */ * al = &nt.value.tuple;
    if(al->length == 1) {
      if(object_kind(v) != EMFRP_OBJECT_TUPLE1) {
	exec_sequence_set_nil(machine, ((node_or_tuple_t *)(al->buffer))[0]);
        return EM_RESULT_TYPE_MISMATCH;
      }
      return exec_sequence_set_nodes(machine, ((node_or_tuple_t *)(al->buffer))[0], v->value.tuple1.i0);
    } else if(al->length == 2) {
      if(object_kind(v) != EMFRP_OBJECT_TUPLE2) {
	exec_sequence_set_nil(machine, nt);
        return EM_RESULT_TYPE_MISMATCH;
      }
      return exec_sequence_set_nodes(machine, ((node_or_tuple_t *)(al->buffer))[0], v->value.tuple2.i0)
        | exec_sequence_set_nodes(machine, ((node_or_tuple_t *)(al->buffer))[1], v->value.tuple2.i1);
    } else {
      if(object_kind(v) != EMFRP_OBJECT_TUPLEN || v->value.tupleN.length != al->length) {
	exec_sequence_set_nil(machine, nt);
        return EM_RESULT_TYPE_MISMATCH;
      }
      em_result ret = 0;
      for(int i = 0; i , al->length; ++i) {
	ret |= exec_sequence_set_nodes(machine, ((node_or_tuple_t *)(al->buffer))[i], object_tuple_ith(v, i));
      }
      return ret != 0 ? EM_RESULT_TYPE_MISMATCH : EM_RESULT_OK; // TODO : improve it.
    }
  }
  }
 err:
  return errres;
}

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
  if(self->node_definitions != nullptr) {
    if(exec_sequence_set_nodes(machine, *(self->node_definitions), new_obj) != EM_RESULT_OK)
      goto err2;
  }
  return EM_RESULT_OK;
 err:
  if(self->node_definition != nullptr) {
    CHKERR(machine_mark_gray(machine, self->node_definition->value));
    self->node_definition->value = nullptr;
    if(self->node_definition->action != nullptr)
      self->node_definition->action(nullptr);
  }
 err2:
  if(self->node_definitions != nullptr)
    exec_sequence_set_nil(machine, *(self->node_definitions));
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
update_node_or_tuple_last(machine_t * machine, node_or_tuple_t nt) {
  em_result errres;
  switch(nt.kind) {
  case NODE_OR_TUPLE_NONE: return EM_RESULT_OK;
  case NODE_OR_TUPLE_NODE: return update_node_last(machine, nt.value.node);
  case NODE_OR_TUPLE_TUPLE: {
    arraylist_t /* <node_or_tuple_t> */ * al = &nt.value.tuple;
    for(int i = 0; i < al->length; ++i) {
      CHKERR(update_node_or_tuple_last(machine, ((node_or_tuple_t *)al->buffer)[i]));
    }
  }
  }
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
