/** -------------------------------------------
 * @file   exec_sequence_t.c
 * @brief  Execution Sequence.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/26
 ------------------------------------------- */
#include "vm/exec_sequence_t.h"
#include "vm/exec.h"
#include "vm/gc.h"
#include "vm/variable_t.h"
#include "vm/machine.h"
#include <stdio.h>

em_result
get_dependencies_ast(machine_t * machine, parser_expression_t * v, list_t /*<string_t *>*/ ** out)
{
  em_result errres = EM_RESULT_OK;
  if(EXPR_KIND_IS_INTEGER(v) || EXPR_KIND_IS_BOOLEAN(v)) return EM_RESULT_OK;
  if(v->kind == EXPR_KIND_IDENTIFIER) {
    string_t * s = &(v->value.identifier);
    object_t * ignore;
    if(!variable_table_lookup(machine->variable_table, &ignore, s)) {
      CHKERR(list_add2(out, string_t *, &s));
    } else
      return EM_RESULT_OK;
  } else if(EXPR_KIND_IS_BIN_OP(v)) {
    CHKERR(get_dependencies_ast(machine, v->value.binary.lhs, out));
    CHKERR(get_dependencies_ast(machine, v->value.binary.rhs, out));
  } else
    switch(v->kind) {
      case EXPR_KIND_TUPLE:
        for(parser_expression_tuple_list_t * li = &(v->value.tuple); li != nullptr; li = li->next)
          CHKERR(get_dependencies_ast(machine, li->value, out));
        break;
      case EXPR_KIND_IF:
        CHKERR(get_dependencies_ast(machine, v->value.ifthenelse.cond, out));
        CHKERR(get_dependencies_ast(machine, v->value.ifthenelse.then, out));
        CHKERR(get_dependencies_ast(machine, v->value.ifthenelse.otherwise, out));
        break;
      case EXPR_KIND_FUNCCALL:
        CHKERR(get_dependencies_ast(machine, v->value.funccall.callee, out));
        if(v->value.funccall.arguments.value != nullptr) {
          for(parser_expression_tuple_list_t * li = &(v->value.funccall.arguments); li != nullptr;
              li                                  = li->next)
            CHKERR(get_dependencies_ast(machine, li->value, out));
        }
        break;
      default:
        break;
    }
  // return EM_RESULT_OK;
err:
  return errres;
}

bool
check_depends_on_ast(parser_expression_t * v, string_t * str)
{
  if(EXPR_KIND_IS_INTEGER(v) || EXPR_KIND_IS_BOOLEAN(v)) return false;
  if(v->kind == EXPR_KIND_IDENTIFIER)
    return string_compare(&(v->value.identifier), str);
  else if(EXPR_KIND_IS_BIN_OP(v))
    return check_depends_on_ast(v->value.binary.lhs, str)
        || check_depends_on_ast(v->value.binary.rhs, str);
  else
    switch(v->kind) {
      case EXPR_KIND_TUPLE:
        for(parser_expression_tuple_list_t * li = &(v->value.tuple); li != nullptr; li = li->next)
          if(check_depends_on_ast(li->value, str)) return true;
        return false;
      case EXPR_KIND_IF:
        return check_depends_on_ast(v->value.ifthenelse.cond, str)
            || check_depends_on_ast(v->value.ifthenelse.then, str)
            || check_depends_on_ast(v->value.ifthenelse.otherwise, str);
      case EXPR_KIND_FUNCCALL:
        if(check_depends_on_ast(v->value.funccall.callee, str)) return true;
        if(v->value.funccall.arguments.value != nullptr) {
          for(parser_expression_tuple_list_t * li = &(v->value.funccall.arguments); li != nullptr;
              li                                  = li->next)
            if(check_depends_on_ast(li->value, str)) return true;
        }
        return false;
      default:
        break;
    }
  return false;
}

typedef struct topo_t
{
  list_t * /*<exec_sequence_t>*/ val;
  list_t * /*<string_t*>*/       references;
} topo_t;

bool
go_topological_sort(string_t * dependency, node_or_tuple_t * nt)
{
  if(nt == nullptr) return false;
  switch(nt->kind) {
    case NODE_OR_TUPLE_NONE:
      return false;
    case NODE_OR_TUPLE_NODE: {
      if(nt->value.node == nullptr) return false;
      return string_compare(dependency, &(nt->value.node->name));
    }
    case NODE_OR_TUPLE_TUPLE:
      for(int i = 0; i < nt->value.tuple.length; ++i)
        if(go_topological_sort(dependency, &(((node_or_tuple_t *)(nt->value.tuple.buffer))[i])))
          return true;
      return false;
    default:
      return false;
  }
}

em_result
topological_sort(
  machine_t * machine, list_t /*<exec_sequence_t>*/ ** es, list_t /*<exec_sequence_t>*/ *** last_es)
{
  em_result errres = EM_RESULT_OK;
  if(*es == nullptr) return EM_RESULT_OK;
  queue_t /*<topo_t>*/ ts;
  CHKERR(queue_default(&ts));
  queue_t /*<topo_t>*/ empty;
  CHKERR(queue_default(&empty));
  for(list_t /*<exec_sequence_t>*/ * i = *es; i != nullptr; i = LIST_NEXT(i)) {
    topo_t            v = {i, nullptr};
    exec_sequence_t * e = (exec_sequence_t *)&(i->value);
    if(e->program_kind == EMFRP_PROGRAM_KIND_AST)
      CHKERR(get_dependencies_ast(machine, e->program.ast, &(v.references)));
    CHKERR(queue_enqueue(v.references == nullptr ? &empty : &ts, sizeof(topo_t), &v));
  }
  for(list_t * li = empty.head; li != nullptr; li = LIST_NEXT(li)) {
    topo_t *          li_v   = (topo_t *)&(li->value);
    exec_sequence_t * li_v_v = (exec_sequence_t *)&(li_v->val->value);
    for(list_t ** lli_ = &(ts.head); *lli_ != nullptr;) {
      list_t * lli   = *lli_;
      topo_t * lli_v = (topo_t *)&(lli->value);
      for(list_t ** /*<string_t *>*/ refs = &(lli_v->references); *refs != nullptr;) {
        if(
          (li_v_v->node_definition != nullptr
           && string_compare(&(li_v_v->node_definition->name), (string_t *)((*refs)->value)))
          || go_topological_sort((string_t *)((*refs)->value), li_v_v->node_definitions)) {
          list_t * ne = (*refs)->next;
          em_free(*refs);
          *refs = ne;
        } else
          refs = &((*refs)->next);
      }
      if(lli_v->references == nullptr) {
        list_t * ne   = lli->next;
        *(empty.last) = lli;
        empty.last    = &(lli->next);
        lli->next     = nullptr;
        *lli_         = ne;
      } else
        lli_ = &(lli->next);
    }
  }

  if(ts.head == nullptr) {  // SUCCESS
    list_t ** las = es;
    for(list_t * cur = empty.head; cur != nullptr; cur = cur->next) {
      *las = ((topo_t *)(&(cur->value)))->val;
      las  = &((*las)->next);
    }
    *(last_es) = las;
  }

  list_t * cur = ts.head;
  while(cur != nullptr) {
    list_t * f = cur;
    cur        = cur->next;
    list_free(&(((topo_t *)(&(f->value)))->references));
    em_free(f);
  }
  cur = empty.head;
  while(cur != nullptr) {
    list_t * f = cur;
    cur        = cur->next;
    list_free(&(((topo_t *)(&(f->value)))->references));
    em_free(f);
  }

  return ts.head == nullptr ? EM_RESULT_OK : EM_RESULT_CYCLIC_REFERENCE;
err:  // TODO: Freeing.
  return errres;
}

em_result
exec_sequence_set_nil(machine_t * machine, node_or_tuple_t * nt)
{
  em_result errres = EM_RESULT_OK;
  switch(nt->kind) {
    case NODE_OR_TUPLE_NONE:
      return EM_RESULT_OK;
    case NODE_OR_TUPLE_NODE: {
      node_t * n = nt->value.node;
      CHKERR(machine_mark_gray(machine, n->value));
      n->value = nullptr;
      if(n->action != nullptr) n->action(nullptr);
      break;
    }
    case NODE_OR_TUPLE_TUPLE: {
      arraylist_t /* <node_or_tuple_t> */ * al = &(nt->value.tuple);
      for(int i = 0; i < al->length; ++i)
        CHKERR(exec_sequence_set_nil(machine, &(((node_or_tuple_t *)(al->buffer))[i])));
      break;
    }
      errres = EM_RESULT_CYCLIC_REFERENCE;
  }
  return EM_RESULT_OK;
err:
  return errres;
}

em_result
exec_sequence_set_nodes(machine_t * machine, node_or_tuple_t * nt, object_t * v)
{
  em_result errres = EM_RESULT_OK;
  if(v == nullptr) return exec_sequence_set_nil(machine, nt);
  switch(nt->kind) {
    case NODE_OR_TUPLE_NONE:
      return EM_RESULT_OK;
    case NODE_OR_TUPLE_NODE: {
      node_t * n = nt->value.node;
      if(n == nullptr) return EM_RESULT_OK;
      CHKERR(machine_mark_gray(machine, n->value));
      n->value = v;
      if(n->action != nullptr) n->action(v);
      return EM_RESULT_OK;
    }
    case NODE_OR_TUPLE_TUPLE: {
      arraylist_t /* <node_or_tuple_t> */ * al = &(nt->value.tuple);
      if(al->length == 0) return EM_RESULT_TYPE_MISMATCH;
      if(al->length == 1) {
        if(!object_is_pointer(v) || object_kind(v) != EMFRP_OBJECT_TUPLE1) {
          exec_sequence_set_nil(machine, nt);
          return EM_RESULT_TYPE_MISMATCH;
        }
        return exec_sequence_set_nodes(
          machine, &(((node_or_tuple_t *)(al->buffer))[0]), v->value.tuple1.i0);
      } else if(al->length == 2) {
        if(!object_is_pointer(v) || object_kind(v) != EMFRP_OBJECT_TUPLE2) {
          exec_sequence_set_nil(machine, nt);
          return EM_RESULT_TYPE_MISMATCH;
        }
        return exec_sequence_set_nodes(
                 machine, &(((node_or_tuple_t *)(al->buffer))[0]), v->value.tuple2.i0)
             | exec_sequence_set_nodes(
                 machine, &(((node_or_tuple_t *)(al->buffer))[1]), v->value.tuple2.i1);
      } else {
        if(
          !object_is_pointer(v) || object_kind(v) != EMFRP_OBJECT_TUPLEN
          || v->value.tupleN.length != al->length) {
          exec_sequence_set_nil(machine, nt);
          return EM_RESULT_TYPE_MISMATCH;
        }
        em_result ret = 0;
        for(int i = 0; i < al->length; ++i) {
          ret |= exec_sequence_set_nodes(
            machine, &(((node_or_tuple_t *)(al->buffer))[i]), object_tuple_ith(v, i));
        }
        return ret != 0 ? EM_RESULT_TYPE_MISMATCH : EM_RESULT_OK;  // TODO : improve it.
      }
    }
  }
err:
  return errres;
}

em_result
exec_sequence_update_value_given_object(machine_t * machine, exec_sequence_t * self, object_t * obj)
{
  em_result errres = EM_RESULT_OK;
  if(self->node_definition != nullptr) {
    CHKERR(machine_mark_gray(machine, self->node_definition->value));
    self->node_definition->value = obj;
    if(self->node_definition->action != nullptr) self->node_definition->action(obj);
  }
  if(
    self->node_definitions != nullptr
    && exec_sequence_set_nodes(machine, self->node_definitions, obj) != EM_RESULT_OK)
    goto err2;
  return EM_RESULT_OK;
err:
  if(self->node_definition != nullptr) {
    CHKERR(machine_mark_gray(machine, self->node_definition->value));
    self->node_definition->value = nullptr;
    if(self->node_definition->action != nullptr) self->node_definition->action(nullptr);
  }
err2:
  if(self->node_definitions != nullptr) exec_sequence_set_nil(machine, self->node_definitions);
  return errres;
}

em_result
exec_sequence_update_value(machine_t * machine, exec_sequence_t * self)
{
  em_result  errres;
  object_t * new_obj = nullptr;
  switch(exec_sequence_program_kind(self)) {
    case EMFRP_PROGRAM_KIND_AST:
      CHKERR(exec_ast(machine, self->program.ast, &new_obj));
      break;
    case EMFRP_PROGRAM_KIND_CALLBACK:
      new_obj = self->program.callback();
      break;
    case EMFRP_PROGRAM_KIND_NOTHING:
      return EM_RESULT_OK;
  }
  errres = exec_sequence_update_value_given_object(machine, self, new_obj);
  if(errres != EM_RESULT_OK) goto err;
  exec_sequence_unmark_lastfailed(self);
  return errres;
err:
#if __ESP_IDF
  return errres;
#endif
  if(errres != EM_RESULT_OK && !exec_sequence_marked_lastfailed(self)) {
    exec_sequence_mark_lastfailed(self);
    printf("The execution of ");
    if(self->node_definitions != nullptr)
      node_or_tuple_debug_print(self->node_definitions);
    else
      printf("%s", self->node_definition->name.buffer);
    printf(" is failed: %s\n", EM_RESULT_STR_TABLE[errres]);
  }
  return errres;
}

em_result
update_node_last(struct machine_t * machine, node_t * n)
{
  em_result errres;
  if(n == nullptr) return EM_RESULT_OK;
  CHKERR(machine_mark_gray(machine, n->last));
  n->last = n->value;
  return EM_RESULT_OK;
err:
  return errres;
}

em_result
update_node_or_tuple_last(machine_t * machine, node_or_tuple_t nt)
{
  em_result errres;
  switch(nt.kind) {
    case NODE_OR_TUPLE_NONE:
      return EM_RESULT_OK;
    case NODE_OR_TUPLE_NODE:
      return update_node_last(machine, nt.value.node);
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
exec_sequence_update_last(struct machine_t * machine, exec_sequence_t * self)
{
  em_result errres;
  CHKERR(update_node_last(machine, self->node_definition));
  return EM_RESULT_OK;
err:
  return errres;
}

bool
node_or_tuple_t_compact(node_or_tuple_t * nt)
{
  switch(nt->kind) {
    case NODE_OR_TUPLE_NONE:
      return true;
    case NODE_OR_TUPLE_NODE:
      if(nt->value.node == nullptr) {
        nt->kind = NODE_OR_TUPLE_NONE;
        return true;
      } else
        return false;
    case NODE_OR_TUPLE_TUPLE: {
      bool result = true;
      for(int i = 0; i < nt->value.tuple.length; ++i) {
        node_or_tuple_t * nn = &(((node_or_tuple_t *)nt->value.tuple.buffer)[i]);
        result &= node_or_tuple_t_compact(nn);
      }
      if(result) {
        arraylist_free(&(nt->value.tuple));
        nt->kind = NODE_OR_TUPLE_NONE;
      }
      return result;
    }
    default:
      DEBUGBREAK;
      return false;
  }
}

bool
exec_sequence_compact(exec_sequence_t * es)
{
  if(es->node_definitions == nullptr) goto end;
  if(node_or_tuple_t_compact(es->node_definitions)) {
    free(es->node_definitions);
    es->node_definitions = nullptr;
  }
end:
  return (es->node_definition == nullptr && es->node_definitions == nullptr);
}

void
exec_sequence_free(exec_sequence_t * es)
{
#if DEBUG
  if(es->node_definitions != nullptr) DEBUGBREAK;
#endif
  if(exec_sequence_program_kind(es) == EMFRP_PROGRAM_KIND_AST)
    parser_expression_free(es->program.ast);
}

#include <stdio.h>

void
node_or_tuple_debug_print(node_or_tuple_t * nt)
{
  switch(nt->kind) {
    case NODE_OR_TUPLE_NONE:
      printf("*");
      break;
    case NODE_OR_TUPLE_NODE:
      printf("%s", nt->value.node->name.buffer);
      break;
    case NODE_OR_TUPLE_TUPLE:
      printf("(");
      {  //if(nt->value.tuple.length > 0) {
        node_or_tuple_debug_print(&(((node_or_tuple_t *)(nt->value.tuple.buffer))[0]));
        for(int i = 1; i < nt->value.tuple.length; ++i) {
          printf(", ");
          node_or_tuple_debug_print(&(((node_or_tuple_t *)(nt->value.tuple.buffer))[i]));
        }
      }
      printf(")");
      break;
  }
}
