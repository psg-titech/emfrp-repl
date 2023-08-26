/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point for UNIX-like systems.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/15
 ------------------------------------------- */
#include <stdio.h>
#include <editline/readline.h>
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"
#include "vm/exec_sequence_t.h"

int
initialize_console(void)
{
  rl_readline_name = "emfrp-repl";
}

em_result
read_line(string_t * out)
{
  char * line;
  line = readline("emfrp: ");
  if(line == nullptr) return EM_RESULT_UNKNOWN_ERR;
  string_new1(out, line);
  return EM_RESULT_OK;
}

int
main(void)
{
  string_t            line;
  parser_reader_t     parser_reader;
  parser_toplevel_t * parsed;
  machine_t           m;
  string_null(&line);
  machine_new(&m);
  initialize_console();
  while(true) {
    string_free(&line);
    if(read_line(&line) != EM_RESULT_OK) {
      printf("ERROR: Console Read.\nabort.\n");
      return 1;
    }
    if(line.length == 0) continue;
    parser_reader_new(&parser_reader, &line);
    if(line.length == 14 && strncmp(line.buffer, "exec_iteration", 14) == 0) {
      printf("Status: %s\n", EM_RESULT_STR_TABLE[machine_indicate(&m, nullptr, 0)]);
      continue;
    }
    if(line.length == 4 && strncmp(line.buffer, "exit", 4) == 0) return 0;
    parser_context_t * ctx = parser_create(&parser_reader);
    if(!parser_parse(ctx, (void **)&parsed)) {
      object_t * o = nullptr;
      parser_toplevel_print(parsed);
      puts("");
      em_result res = machine_exec(&m, parsed, &o);
      // We have to think free_deep or free_shallow.
      if(res != EM_RESULT_OK) {
        printf("machine_exec failure(%d): %s\n", res, EM_RESULT_STR_TABLE[res]);
        printf("%s\n", EM_RESULT_STR_TABLE[res]);
        parser_toplevel_free_deep(parsed);
      } else {
        parser_toplevel_free_shallow(parsed);
        fputs("OK, ", stdout);
        object_print(o);
        puts("");
      }
      machine_debug_print_definitions(&m);
    }
    parser_destroy(ctx);
  }
  return 0;
}
