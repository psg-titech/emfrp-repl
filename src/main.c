/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/26
 ------------------------------------------- */

#include <stdio.h>
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm.h"
int main(void) {
  string_t line;
  parser_reader_t parser_reader;
  parser_node_t * parsed_node;
  object_t * result_object;
  machine_t m;
  string_null(&line);
  initialize_console();
  read_line(&line);
  parser_reader_new(&parser_reader, &line);
  parser_context_t *ctx = parser_create(&parser_reader);
  if(!parser_parse(ctx, &parsed_node)) {
    parser_node_print(parsed_node);
    machine_new(&m);
    em_result res = exec_ast(&m, parsed_node->expression,  &result_object);
    printf("%d, %d\n", res,
	   object_get_integer(result_object));
  }
  return 0;
}
