/** -------------------------------------------
 * @file   emfrp.c
 * @brief  Emfrp Main Functions
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/18
 ------------------------------------------- */
#include "vm/machine.h"
#include "vm/object_t.h"
#include "emmem.h"
#include "emfrp_parser.h"
#include "emfrp.h"
#include <stdio.h>
typedef struct emfrp_t
{
  machine_t * machine;
} emfrp_t;

EM_EXPORTDECL em_result
emfrp_create(emfrp_t ** out)
{
  em_result errres = EM_RESULT_OK;
  emfrp_t * ret    = nullptr;
  CHKERR(em_malloc((void **)&ret, sizeof(emfrp_t)));
  ret->machine = nullptr;
  CHKERR(em_malloc((void **)&(ret->machine), sizeof(machine_t)));
  machine_new(ret->machine);
  *out = ret;
  return EM_RESULT_OK;
err:
  if(ret != nullptr && ret->machine != nullptr) em_free(ret->machine);
  if(ret != nullptr) em_free(ret);
  return errres;
}

EM_EXPORTDECL em_result
emfrp_repl(emfrp_t * self, const char * str, object_t ** out)
{
  em_result           errres = EM_RESULT_OK;
  parser_reader_t     parser_reader;
  parser_toplevel_t * parsed;
  string_t            line;
  string_new1(&line, strdup(str));
  parser_reader_new(&parser_reader, &line);
  parser_context_t * ctx = parser_create(&parser_reader);
  if(!parser_parse(ctx, (void **)&parsed)) {
    parser_toplevel_print(parsed);
    printf("\n");
    errres = machine_exec(self->machine, parsed, out);
    if(errres != EM_RESULT_OK)
      parser_toplevel_free_deep(parsed);
    else
      parser_toplevel_free_shallow(parsed);
  } else
    errres = EM_RESULT_PARSE_ERROR;
  parser_destroy(ctx);
  if(errres == EM_RESULT_OK) machine_debug_print_definitions(self->machine);
  return errres;
}

EM_EXPORTDECL em_result
emfrp_add_input_node(emfrp_t * self, char * node_name, em_input_callback callback)
{
  em_result errres = EM_RESULT_OK;
  string_t  s, s_dup;
  string_new1(&s, node_name);
  CHKERR(string_copy(&s_dup, &s));
  return machine_add_node_callback(self->machine, s_dup, callback);
err:
  return errres;
}

EM_EXPORTDECL em_result
emfrp_add_output_node(emfrp_t * self, char * node_name, em_output_callback callback)
{
  em_result errres = EM_RESULT_OK;
  string_t  s, s_dup;
  string_new1(&s, node_name);
  CHKERR(string_copy(&s_dup, &s));
  return machine_add_output_node(self->machine, s_dup, callback);
err:
  return errres;
}

EM_EXPORTDECL em_result
emfrp_set_node_value(emfrp_t * self, char * node_name, em_object_t * v)
{
  string_t s;
  string_new1(&s, node_name);
  return machine_set_value_of_node(self->machine, &s, v);
}

EM_EXPORTDECL em_result
emfrp_update(emfrp_t * self)
{
  return machine_indicate(self->machine, nullptr, 0);
}

EM_EXPORTDECL em_object_t *
emfrp_create_int_object(int32_t num)
{
  em_object_t * output = nullptr;
  object_new_int(&output, num);
  return output;
}

EM_EXPORTDECL em_object_t *
emfrp_get_true_object(void)
{
  return &object_true;
}
EM_EXPORTDECL em_object_t *
emfrp_get_false_object(void)
{
  return &object_false;
}

EM_EXPORTDECL int32_t
emfrp_get_integer(em_object_t * v)
{
  return object_get_integer(v);
}

EM_EXPORTDECL void
emfrp_print_object(em_object_t * v)
{
  object_print(v);
}
