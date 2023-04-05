/** -------------------------------------------
 * @file   console.esp32.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/18
 ------------------------------------------- */
#if defined(__ESP_IDF__)
#include "hal/console.h"
#include "string_t.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "soc/uart_struct.h"
#include <unistd.h>
#include <stdio.h>

// linenoise, fgets, and esp_console are completely garbages!

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
  //  printf("%s\n", buffer);
}

static const char * PROMPT = "emfrp : ";

em_result
read_line(string_t * recycle_buffer)
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
  if(recycle_buffer->length != nullptr) string_free(recycle_buffer);
  string_new(recycle_buffer, buf, start);
  return EM_RESULT_OK;
}

#endif
