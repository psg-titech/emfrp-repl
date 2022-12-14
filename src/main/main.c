/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/14
 ------------------------------------------- */

#include <stdio.h>
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"
#include "vm/exec_sequence_t.h"
int main(void) {
  string_t line;
  parser_reader_t parser_reader;
  parser_node_t * parsed_node;
  object_t * result_object;
  machine_t m;
  string_null(&line);
  initialize_console();
  machine_new(&m);
  while(true) {
    string_null(&line);
    read_line(&line);
    if(line.length == 0) continue;
    parser_reader_new(&parser_reader, &line);
    parser_context_t *ctx = parser_create(&parser_reader);
    if(!parser_parse(ctx, (void **)&parsed_node)) {
      exec_sequence_t * es;
      parser_node_print(parsed_node);
      em_result res = machine_add_node_ast(&m, &es, parsed_node);
      if(res != EM_RESULT_OK) {
	printf("add node failure(%d): %s\n", res, EM_RESULT_STR_TABLE[res]);
	parser_node_free_deep(parsed_node);
	goto freeing;
      }
      res = exec_ast(&m, parsed_node->expression,  &result_object);
      // We have to think free_deep or free_shallow.
      if(res != EM_RESULT_OK) {
	printf("%s\n", EM_RESULT_STR_TABLE[res]);
	parser_node_free_deep(parsed_node);
      } else {
	exec_sequence_update_value_given_object(&m, es, result_object);
	parser_node_free_shallow(parsed_node);
	printf("OK, ");
	object_print(result_object);
	printf("\n");
      }
    freeing:
      machine_debug_print_definitions(&m);
    }
    parser_destroy(ctx);
  }
  return 0;
}
