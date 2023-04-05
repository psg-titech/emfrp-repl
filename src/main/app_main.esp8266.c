/** -------------------------------------------
 * @file   app_main.esp8266.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/11
 ------------------------------------------- */
#if defined(__ESP_8266__)

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
//#include "driver/gpio.h"
#include "hal/console.h"
#include "emfrp_parser.h"
#include "ast.h"
#include "vm/machine.h"
#include "vm/exec.h"

machine_t m;

void
mainTask(void)
{
  string_t            line;
  parser_reader_t     parser_reader;
  parser_toplevel_t * parsed;
  string_null(&line);
  initialize_console();
  machine_new(&m);
  printf("Emfrp REPL on ESP8266.\n");
  machine_debug_print_definitions(&m);
  while(true) {
    string_null(&line);
    read_line(&line);
    if(line.length == 0) {
      // It may be noise.
      vTaskDelay(100);
      continue;
    }
    parser_reader_new(&parser_reader, &line);
    parser_context_t * ctx = parser_create(&parser_reader);
    if(!parser_parse(ctx, (void **)&parsed)) {
      object_t * o = nullptr;
      //      printf("Heap free size: %d\n", esp_get_free_heap_size());
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
    //    printf("Heap free size: %d\n", esp_get_free_heap_size());
  }
  while(1) {
    vTaskDelay(100000);
  }
}

void
app_main()
{
  xTaskCreate(mainTask, "main_task", 16384, nullptr, 10, nullptr);
}

#endif
