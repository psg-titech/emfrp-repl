/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/2/15
 ------------------------------------------- */
#if !defined(__MBED__)
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
  parser_toplevel_t * parsed;
  machine_t m;
  string_null(&line);
  initialize_console();
  machine_new(&m);
  while(true) {
    string_null(&line);
    read_line(&line);
    if(line.length == 0) continue;
    parser_reader_new(&parser_reader, &line);
    if(line.length == 14 && strncmp(line.buffer, "exec_iteration", 14) == 0) {
      printf("Result Code: %s\n", EM_RESULT_STR_TABLE[machine_indicate(&m, nullptr, 0)]);
      em_free(line.buffer);
      continue;
    }
    if(line.length == 4 && strncmp(line.buffer, "exit", 4) == 0)
      return 0;
    parser_context_t *ctx = parser_create(&parser_reader);
    if(!parser_parse(ctx, (void **)&parsed)) {
      object_t * o = nullptr;
      parser_toplevel_print(parsed);
      printf("\n");
      em_result res = machine_exec(&m, parsed, &o);
      // We have to think free_deep or free_shallow.
      if(res != EM_RESULT_OK) {
	printf("machine_exec failure(%d): %s\n", res, EM_RESULT_STR_TABLE[res]);
	printf("%s\n", EM_RESULT_STR_TABLE[res]);
	parser_toplevel_free_deep(parsed);
      } else {
	parser_toplevel_free_shallow(parsed);
	printf("OK, ");
	object_print(o);
	printf("\n");
      }
      machine_debug_print_definitions(&m);
    }
    parser_destroy(ctx);
  }
  return 0;
}
#endif