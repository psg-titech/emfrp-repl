/** -------------------------------------------
 * @file   rpi_main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/6
 ------------------------------------------- */

#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"

const uint LED_PIN = 25;
const uint SWITCH_PIN = 21;
machine_t m;
int main() {

  bi_decl(bi_program_description("Emfrp-REPL"));
  bi_decl(bi_1pin_with_name(LED_PIN, "On-board LED"));
  bi_decl(bi_1pin_with_name(SWITCH_PIN, "Switch"));
  
  // stdio_init_all();

  // gpio_init(LED_PIN); gpio_init(SWITCH_PIN);
  // gpio_set_dir(LED_PIN, GPIO_OUT);
  // gpio_set_dir(SWITCH_PIN,GPIO_IN);
  // while(1) {
  //   int i = gpio_get(SWITCH_PIN);
  //   gpio_put(LED_PIN, i);
  //   sleep_ms(250);
  //   //    gpio_put(LED_PIN, 1);
  //   //    puts("Hello World\n");
  //   //    sleep_ms(1000);
  // }

  
  string_t line;
  parser_reader_t parser_reader;
  parser_toplevel_t * parsed;
  object_t * result_object;
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
    parser_context_t *ctx = parser_create(&parser_reader);
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
  while(1){       sleep_ms(1000);}
  
}
