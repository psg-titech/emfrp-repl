/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */

#include <stdio.h>
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
int main(void) {
  string_t line;
  parser_reader_t parser_reader;
  parser_node_t * parsed_node;
  string_null(&line);
  initialize_console();
  read_line(&line);
  parser_reader_new(&parser_reader, &line);
  parser_context_t *ctx = parser_create(&parser_reader);
  if(!parser_parse(ctx, &parsed_node))
    parser_node_print(parsed_node);
  return 0;
}
