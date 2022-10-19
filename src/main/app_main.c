/** -------------------------------------------
 * @file   app_main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/1
 ------------------------------------------- */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"

machine_t m;

object_t * gpio_input(void) {
  object_t * ret = nullptr;
  object_new_int(&ret, gpio_get_level(4));
  return ret;
}

void gpio_output(object_t * obj) {
  gpio_set_level(5, object_get_integer(obj));
}

void interruption(void * arg) {
  string_t s;
  string_new1(&s, "switch");
  machine_set_value_of_node(&m, &s, gpio_input());
  machine_indicate(&m, &s, 1);
}

void setup_gpio_test(void) {
  gpio_config_t g = {0};
  g.intr_type = GPIO_INTR_DISABLE;
  g.mode = GPIO_MODE_OUTPUT;
  g.pin_bit_mask = 1ULL << 5; // GPIO5
  g.pull_down_en = 0;
  g.pull_up_en = 0;
  gpio_config(&g);
  g.intr_type = GPIO_INTR_ANYEDGE;
  g.pin_bit_mask = 1ULL << 4; // GPIO4
  g.mode = GPIO_MODE_INPUT;
  g.pull_up_en = 1;
  gpio_config(&g);
  gpio_install_isr_service(0);
  string_t s;
  string_new1(&s, strdup("switch"));
  machine_add_node_callback(&m, s, gpio_input);
  string_new1(&s, strdup("led"));
  machine_add_output_node(&m, s, gpio_output);
  gpio_isr_handler_add(4, interruption, nullptr);
}

void mainTask(void) {
  string_t line;
  parser_reader_t parser_reader;
  parser_node_t * parsed_node;
  object_t * result_object;
  string_null(&line);
  initialize_console();
  machine_new(&m);
  setup_gpio_test();
  printf("Emfrp REPL on ESP32.\n");
      machine_debug_print_definitions(&m);
      gpio_set_level(5, 1);
  while(true) {
    string_null(&line);
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
      // We have to think parsed_node->name is freed or not.
      if (parsed_node->init_expression == nullptr)
        machine_set_value_of_node(&m, &(parsed_node->name), result_object);
      else
        object_free(result_object);
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
  xTaskCreate(mainTask, "main_task", 16384, nullptr, 10, nullptr);
}
