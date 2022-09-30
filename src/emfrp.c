/** -------------------------------------------
 * @file   emfrp.c
 * @brief  Emfrp Main Functions
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/30
 ------------------------------------------- */
#include "vm/machine.h"
#include "vm/object_t.h"
#include "emmem.h"
#include "emfrp_parser.h"
#include "emfrp.h"

typedef struct emfrp_t{
  machine_t * machine;
} emfrp_t;

emfrp_t * EM_EXPORTDECL
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

bool EM_EXPORTDECL
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
 fail:
  parser_destroy(ctx);
  return true;
}

bool EM_EXPORTDECL
emfrp_add_input_node_definition(emfrp_t * self, char * node_name, em_callback callback) {
  string_t s;
  string_new1(&s, node_name);
  return machine_add_node_callback(self->machine, s, callback) != EM_RESULT_OK;
} 
bool EM_EXPORTDECL emfrp_indicate_node_update(emfrp_t * self, char * node_name, em_object_t * value) {
  return false;
}
bool EM_EXPORTDECL emfrp_add_output_node_definition(emfrp_t * self, char * node_name, em_callback callback) {
  return false;
}
