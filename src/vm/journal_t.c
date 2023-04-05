/** -------------------------------------------
 * @file   journal_t.c
 * @brief  Journaling on redefining nodes.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/14
 ------------------------------------------- */
#include "vm/journal_t.h"
#include "vm/exec_sequence_t.h"

em_result
remove_the_node(journal_t ** out, exec_sequence_t * es, node_t ** where_placed)
{
  em_result   errres = EM_RESULT_OK;
  journal_t * nj     = nullptr;
  CHKERR(em_malloc((void **)&nj, sizeof(journal_t)));
  nj->next         = *out;
  nj->where_placed = where_placed;
  nj->what         = *where_placed;
  *out             = nj;
  *where_placed    = nullptr;
  exec_sequence_mark_modified(es);
  //  return EM_RESULT_OK;
err:
  return errres;
}

em_result
go_remove_defined_node(journal_t ** out, exec_sequence_t * es, node_t * node, node_or_tuple_t * nt)
{
  em_result errres = EM_RESULT_OK;
  switch(nt->kind) {
    case NODE_OR_TUPLE_NONE:
      return EM_RESULT_OK;
    case NODE_OR_TUPLE_NODE:
      return nt->value.node == node ? remove_the_node(out, es, &(nt->value.node)) : EM_RESULT_OK;
    case NODE_OR_TUPLE_TUPLE:
      for(int i = 0; i < nt->value.tuple.length; ++i) {
        CHKERR(go_remove_defined_node(
          out, es, node, &(((node_or_tuple_t *)(nt->value.tuple.buffer))[i])));
      }
      return EM_RESULT_OK;
  }
err:
  return errres;
}

em_result
remove_defined_node(list_t /*<exec_sequence_t>*/ * exec_sequences, journal_t ** out, node_t * node)
{
  em_result errres = EM_RESULT_OK;
  if(exec_sequences == nullptr || node == nullptr) return EM_RESULT_OK;
  for(list_t /*<exec_sequence_t>*/ * cur = exec_sequences; cur != nullptr; cur = LIST_NEXT(cur)) {
    exec_sequence_t * es = (exec_sequence_t *)&(cur->value);
    if(es->node_definition == node) return remove_the_node(out, es, &(es->node_definition));
    if(es->node_definitions != nullptr) {
      journal_t * out2 = *out;
      CHKERR(go_remove_defined_node(&out2, es, node, es->node_definitions));
      if(out2 != *out) {  // Removed!
        *out = out2;
        return EM_RESULT_OK;
      }
    }
  }
  //return EM_RESULT_OK;
err:
  return errres;
}

void
revert_from_journal(journal_t * j)
{
  while(j != nullptr) {
    *(j->where_placed) = j->what;
    j                  = j->next;
  }
}

void
journal_free(journal_t ** j)
{
  journal_t * cur = *j;
  while(cur != nullptr) {
    journal_t * next = cur->next;
    free(cur);
    cur = next;
  }
  *j = nullptr;
}
