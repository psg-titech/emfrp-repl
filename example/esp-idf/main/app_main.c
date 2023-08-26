/** -------------------------------------------
 * @file   app_main.c
 * @brief  Emfrp-repl Entry Point for ESP32
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/26
 ------------------------------------------- */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "emfrp.h"

/// Console Functions
/// These will be replaced by fgets or linenoise if they works.
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include <unistd.h>
#include "string_t.h"
#define REPL_PROMPT_NUM UART_NUM_0
static QueueHandle_t uart_event_queue;

static char emfrp_console_buf[4096];
#define buf emfrp_console_buf
void
initialize_console(void)
{
  fclose(stdin);
  uart_config_t uart_config = {
    .baud_rate           = 115200,
    .data_bits           = UART_DATA_8_BITS,
    .parity              = UART_PARITY_DISABLE,
    .stop_bits           = UART_STOP_BITS_1,
    .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
    .source_clk          = UART_SCLK_DEFAULT,
    .rx_flow_ctrl_thresh = 122,
  };
  uart_param_config(REPL_PROMPT_NUM, &uart_config);
  uart_driver_install(REPL_PROMPT_NUM, 512, 512, 10, &uart_event_queue, 0);
  //emfrp_console_buf[0] = '\0';
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
  int len = 0, start = 0;
  printf(PROMPT);
  fflush(stdout);
  while(1) {
    len = uart_read_bytes(REPL_PROMPT_NUM, &buf[start], sizeof(buf) - start, 0);
    if(len == 0) {
      vTaskDelay(3);
      continue;
    }
    int count = len + start;
    for(int i = start; i < count; ++i) {
      // Normaly, CR(\r) represents to go to the new line.
      if(buf[i] == '\n' || buf[i] == '\r') {
        uart_write_bytes(REPL_PROMPT_NUM, &buf[start], i - start + 1);
        start = execute_control_sequence(len, start, buf);
        goto end;
      }
    }
    uart_write_bytes(REPL_PROMPT_NUM, &buf[start], len);
    start = execute_control_sequence(len, start, buf);
  }
end:
  buf[start] = '\0';
  string_new(out, buf, start);
  puts("");
  fflush(stdout);
  return EM_RESULT_OK;
}

/// Console Functions

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
  emfrp_set_node_value(em, "gpio16", gpio_input());
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
    goto fail;
  }
  setup_gpio_test();
  puts("Emfrp REPL on ESP32.");
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
      puts("");
    } else {
      printf("machine_exec failure(%d): %s\n", res, EM_RESULT_STR_TABLE[res]);
    }
    printf("Heap free size: %ld\n", esp_get_free_heap_size());
  }
fail:
  while(1) {
    vTaskDelay(100000);
  }
}

void
app_main()
{
  xTaskCreate(mainTask, "main_task", 16384, nullptr, 10, nullptr);
}
