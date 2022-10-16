/** -------------------------------------------
 * @file   app_main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/15
 ------------------------------------------- */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"

void mainTask(void) {
  string_t line;
  parser_reader_t parser_reader;
  parser_node_t * parsed_node;
  object_t * result_object;
  machine_t m;
  string_null(&line);
  initialize_console();
  machine_new(&m);
  printf("Hello from ESP32.\n");
  while(true) {
    read_line(&line);
    if(line.length == 0) {
      // It may be noise.
      vTaskDelay(100);
      continue;
    }
    parser_reader_new(&parser_reader, &line);
    parser_context_t *ctx = parser_create(&parser_reader);
    if(!parser_parse(ctx, (void **)&parsed_node)) {
      printf("Heap free size: %d\n", esp_get_free_heap_size());
      parser_node_print(parsed_node);
      em_result res = machine_add_node_ast(&m, parsed_node->name, parsed_node->expression, parsed_node->init_expression);
      if(res != EM_RESULT_OK) {
	printf("add node failure: %s\n", EM_RESULT_STR_TABLE[res]);
	goto freeing;
      }
      res = exec_ast(&m, parsed_node->expression,  &result_object);
      printf("%s, %d\n", EM_RESULT_STR_TABLE[res],
	     object_get_integer(result_object));
    freeing:
      machine_debug_print_definitions(&m);
      if(parsed_node->init_expression != nullptr)
	parser_expression_free(parsed_node->init_expression);
      em_free(parsed_node);
    }
    parser_destroy(ctx);
    printf("Heap free size: %d\n", esp_get_free_heap_size());
  }
  while(1){ vTaskDelay(100000); }
}

void app_main() {
  xTaskCreate(mainTask, "main_task", 4096, nullptr, 10, nullptr);
}
