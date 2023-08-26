/** -------------------------------------------
 * @file   rpi_main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/25
 ------------------------------------------- */
#if defined(RPI_PICO)
#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"

/// Console Functions
#include "string_t.h"
#include <unistd.h>
static char emfrp_console_buf[4096];
#define buf emfrp_console_buf
void
initialize_console(void)
{
  stdio_init_all();
}

int
execute_control_sequence(int len, int start, char * buffer)
{
  int count = start + len;
  for(int i = start; i < count; ++i) {
    if(buffer[i] == '\b') {
      char * dst = i == 0 ? &buffer[i] : &buffer[i - 1];
      if(i == count - 1)
        *dst = '\0';
      else
        memmove(dst, &buffer[i + 1], count - i - 1);
      if(i > 0) {
        i -= 2;
        count -= 2;
      } else {
        i--;
        count--;
      }
    }
  }
  return count;
}

static const char * PROMPT = "emfrp : ";

em_result
read_line(string_t * out)
{
  int start = 0;
  printf(PROMPT);
  fflush(stdout);
  while(1) {
    char ch;
    scanf("%c", &ch);
    int count  = 1 + start;
    buf[start] = ch;
    // Normaly, CR(\r) represents to go to the new line.
    printf("%c", buf[start]);
    if(ch == '\n' || ch == '\r') {
      start = execute_control_sequence(1, start, buf);
      goto end;
    }
    start = execute_control_sequence(1, start, buf);
  }
end:
  buf[start] = '\0';
  string_new(out, buf, start);
  return EM_RESULT_OK;
}
/// Console Functions

const uint LED_PIN    = 25;
const uint SWITCH_PIN = 21;
machine_t  m;
int
main()
{
  bi_decl(bi_program_description("Emfrp-REPL"));
  bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
  bi_decl(bi_1pin_with_name(SWITCH_PIN, "Switch"));

  // stdio_init_all();
  string_t            line;
  parser_reader_t     parser_reader;
  parser_toplevel_t * parsed;
  object_t *          result_object;
  string_null(&line);
  initialize_console();
  machine_new(&m);
  printf("Emfrp REPL on ESP32.\n");
  machine_debug_print_definitions(&m);
  while(true) {
    string_null(&line);
    read_line(&line);
    if(line.length == 0) {
      // It may be noise.
      sleep_ms(100);
      continue;
    }
    parser_reader_new(&parser_reader, &line);
    parser_context_t * ctx = parser_create(&parser_reader);
    if(!parser_parse(ctx, (void **)&parsed)) {
      object_t * o = nullptr;
      // printf("Heap free size: %d\n", esp_get_free_heap_size());
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
    // printf("Heap free size: %d\n", esp_get_free_heap_size());
  }
  while(1) {
    sleep_ms(1000);
  }
}

#endif
