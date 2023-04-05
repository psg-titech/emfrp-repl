/** -------------------------------------------
 * @file   app_main.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/18
 ------------------------------------------- */
#if defined(__ESP_IDF__)

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "hal/console.h"
#include "emfrp.h"
emfrp_t * em = nullptr;

em_object_t *
gpio_input(void)
{
  return gpio_get_level(16) ? emfrp_get_true_object() : emfrp_get_false_object();
}

void
gpio_output(em_object_t * obj)
{
  gpio_set_level(17, obj == emfrp_get_false_object() ? 0 : 1);
}

void
interruption(void * arg)
{
  //  emfrp_set_node_value(em, "gpio16", gpio_input());
  //gpio_set_level(17, gpio_get_level(16));
  //return;
  em_result res = emfrp_update(em);
}

void
setup_gpio_test(void)
{
  gpio_config_t g = {0};
  g.intr_type     = GPIO_INTR_DISABLE;
  g.mode          = GPIO_MODE_OUTPUT;
  g.pin_bit_mask  = 1ULL << 17;  // GPIO17
  g.pull_down_en  = 0;
  g.pull_up_en    = 0;
  gpio_config(&g);
  g.intr_type    = GPIO_INTR_ANYEDGE;
  g.pin_bit_mask = 1ULL << 16;  // GPIO16
  g.mode         = GPIO_MODE_INPUT;
  gpio_config(&g);
  gpio_install_isr_service(0);
  emfrp_add_input_node(em, "gpio16", gpio_input);
  emfrp_add_output_node(em, "gpio17", gpio_output);
  gpio_isr_handler_add(16, interruption, nullptr);
}

void
mainTask(void)
{
  em_result res;
  string_t  line;
  string_null(&line);
  initialize_console();
  res = emfrp_create(&em);
  if(res != EM_RESULT_OK) {
    printf("Emfrp creation failure: %s\n", EM_RESULT_STR_TABLE[res]);
    while(true) {
      vTaskDelay(100000);
    }
  }
  setup_gpio_test();
  printf("Emfrp REPL on ESP32.\n");
  printf("Heap free size: %ld\n", esp_get_free_heap_size());
  while(true) {
    string_null(&line);
    read_line(&line);
    if(line.length == 0) {
      // It may be noise.
      vTaskDelay(100);
      continue;
    }
    em_object_t * o = nullptr;
    res             = emfrp_repl(em, line.buffer, &o);
    if(res == EM_RESULT_OK) {
      emfrp_print_object(o);
    } else {
      printf("machine_exec failure(%d): %s\n", res, EM_RESULT_STR_TABLE[res]);
    }
    printf("Heap free size: %ld\n", esp_get_free_heap_size());
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
