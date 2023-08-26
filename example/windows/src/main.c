/** -------------------------------------------
 * @file   main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/13
 ------------------------------------------- */
#include <stdio.h>
#include <Windows.h>
#include "emmem.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"
#include "vm/exec_sequence_t.h"

static HANDLE console_input;

int
initialize_console(void)
{
  console_input = GetStdHandle(STD_INPUT_HANDLE);
}

const DWORD START_SIZE = 64;
const DWORD ADD_SIZE   = 64;

em_result
read_line(string_t * out)
{
  em_result errres = EM_RESULT_OK;
  printf("emfrp: ");
  CHKERR(em_allocarray(&(out->buffer), sizeof(char_t), START_SIZE));
  out->length     = START_SIZE;
  DWORD    readed = 0;
  DWORD    toRead = (DWORD)out->length;
  char_t * buf    = out->buffer;
  while(1) {
    DWORD readChars;
    if(ReadConsole(console_input, &buf[readed], toRead, &readChars, NULL) == 0)
      return EM_RESULT_UNKNOWN_ERR;
    bool hasNewLine = false;
    for(int i = readed; i < readChars + readed; ++i)
      hasNewLine |= buf[i] == '\r';
    readed += readChars;
    if(hasNewLine) break;
    char_t * newBuf = nullptr;
    CHKERR(em_reallocarray(&newBuf, out->buffer, sizeof(char_t), out->length + ADD_SIZE));
    out->length += ADD_SIZE;
    out->buffer = newBuf;
    buf         = newBuf;
    toRead      = ADD_SIZE;
  }
  CHKERR(em_reallocarray(&(out->buffer), out->buffer, sizeof(char_t), readed + 1));
  out->length         = readed;
  out->buffer[readed] = '\0';
  return EM_RESULT_OK;
err:
  return errres;
}

int
main(void)
{
  string_t            line;
  parser_reader_t     parser_reader;
  parser_toplevel_t * parsed;
  machine_t           m;
  string_null(&line);
  initialize_console();
  machine_new(&m);
  while(true) {
    string_free(&line);
    if(read_line(&line) != EM_RESULT_OK) {
      puts("Console Reading Failed.\nAborting.");
      return 1;
    }
    if(line.length == 0) continue;
    parser_reader_new(&parser_reader, &line);
    if(line.length == 14 && strncmp(line.buffer, "exec_iteration", 14) == 0) {
      printf("Status: %s\n", EM_RESULT_STR_TABLE[machine_indicate(&m, nullptr, 0)]);
      em_free(line.buffer);
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
