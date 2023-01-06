/** -------------------------------------------
 * @file   console.rpi.c
 * @brief  Emfrp-repl Entry Point
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/6
 ------------------------------------------- */
#if defined(RPI_PICO)
#include "hal/console.h"
#include "pico/stdlib.h"
#include "string_t.h"
#include <unistd.h>
#include <stdio.h>

// linenoise, fgets, and esp_console are completely garbages!

static char emfrp_console_buf[4096];
#define buf emfrp_console_buf
void
initialize_console(void) {
  stdio_init_all();
}

int
execute_control_sequence(int len, int start, char * buffer) {
  int count = start + len;
  for(int i = start; i < count; ++i) {
    if(buffer[i] == '\b') {
      char * dst = i == 0 ? &buffer[i] : &buffer[i-1];
      if (i == count - 1)
	*dst = '\0';
      else
	memmove(dst, &buffer[i+1], count - i - 1);
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

// This does not work.
em_result
read_line(string_t * recycle_buffer) {
  int len = 0, start = 0;
  printf(PROMPT);
  fflush(stdout);
  while(1) {
    char ch;
    scanf("%c", &ch);
    int count = 1 + start;
    for(int i = start; i < count; ++i) {
      // Normaly, CR(\r) represents to go to the new line.
      if(buf[i] == '\n' || buf[i] == '\r') {
        printf("%c", &buf[start]);
      	start = execute_control_sequence(len, start, buf);
	      goto end;
      }
    }
    printf("%c", &buf[start]);
    start = execute_control_sequence(len, start, buf);
  }
 end:
  buf[start] = '\0';
  string_new(recycle_buffer, buf, start);
  return EM_RESULT_OK;
}

#endif
