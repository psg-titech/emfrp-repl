/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/14
 ------------------------------------------- */

#include <stdio.h>
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"
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
    read_line(&line);
    if(line.length == 0) continue;
    parser_reader_new(&parser_reader, &line);
    parser_context_t *ctx = parser_create(&parser_reader);
    if(!parser_parse(ctx, (void **)&parsed_node)) {
      parser_node_print(parsed_node);
      em_result res = machine_add_node_ast(&m, parsed_node->name, parsed_node->expression, parsed_node->init_expression);
      if(res != EM_RESULT_OK) {
	printf("add node failure: %d\n", res);
	return 0;
      }
      res = exec_ast(&m, parsed_node->expression,  &result_object);
      printf("%d, %d\n", res,
	     object_get_integer(result_object));
      machine_debug_print_definitions(&m);
      parser_expression_free(parsed_node->init_expression);
      em_free(parsed_node);
    }
    parser_destroy(ctx);
  }
  return 0;
}
