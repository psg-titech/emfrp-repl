/** -------------------------------------------
 * @file   emfrp.c
 * @brief  Emfrp Main Functions
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/12
 ------------------------------------------- */
#include "vm/machine.h"
#include "vm/object_t.h"
#include "emmem.h"
#include "emfrp_parser.h"
#include "emfrp.h"

typedef struct emfrp_t{
  machine_t * machine;
} emfrp_t;

EM_EXPORTDECL emfrp_t *
emfrp_create(void) {
  em_result errres;
  emfrp_t * ret = nullptr;
  ret = em_malloc(sizeof(emfrp_t));
  ret->machine = nullptr;
  ret->machine = em_malloc(sizeof(machine_t));
  machine_new(ret->machine);
  return ret;
 err:
  if(ret != nullptr && ret->machine != nullptr)
    em_free(ret->machine);
  if(ret != nullptr)
    em_free(ret);
  return nullptr;
}

EM_EXPORTDECL bool
emfrp_repl(emfrp_t * self, char * str) {
  parser_reader_t parser_reader;
  parser_node_t * parsed_node;
  string_t line;
  string_new1(&line, str);
  parser_reader_new(&parser_reader, &line);
  parser_context_t * ctx = parser_create(&parser_reader);
  if(!parser_parse(ctx, (void **)&parsed_node)) {
    em_result res = machine_add_node_ast(self->machine, parsed_node->name, parsed_node->expression);
    if(res != EM_RESULT_OK)
      goto fail;
    em_free(parsed_node);
  } else
    goto fail;
  parser_destroy(ctx);
  machine_debug_print_definitions(self->machine);
  return false;
 fail:
  parser_destroy(ctx);
  return true;
}

EM_EXPORTDECL bool
emfrp_add_input_node_definition(emfrp_t * self, char * node_name, em_input_node_callback callback) {
  string_t s, s_dup;
  string_new1(&s, node_name);
  if(string_copy(&s_dup, &s) != EM_RESULT_OK) return true;
  return machine_add_node_callback(self->machine, s_dup, callback) != EM_RESULT_OK;
} 

EM_EXPORTDECL bool
emfrp_indicate_node_update(emfrp_t * self, char * node_name, em_object_t * value) {
  string_t s;
  em_result errres;
  string_new1(&s, node_name);
  CHKERR(machine_set_value_of_node(self->machine, &s, value));
  CHKERR(machine_indicate(self->machine, &s, 1));
  return false;
err:
  return true;
}

EM_EXPORTDECL bool
emfrp_add_output_node_definition(emfrp_t * self, char * node_name, em_output_node_callback callback) {
  string_t s, s_dup;
  string_new1(&s, node_name);
  if (string_copy(&s_dup, &s) != EM_RESULT_OK) return true;
  return machine_add_output_node(self->machine, s_dup, callback) != EM_RESULT_OK;
}

EM_EXPORTDECL em_object_t *
emfrp_create_int_object(int32_t num) {
  em_object_t * output = nullptr;
  object_new_int(&output, num);
  return output;
}

EM_EXPORTDECL int32_t
emfrp_get_integer(em_object_t * v) {
    return object_get_integer(v);
}
