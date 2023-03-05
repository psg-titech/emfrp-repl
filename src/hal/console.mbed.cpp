/** -------------------------------------------
 * @file   console.mbed.cpp
 * @brief  Console Implementation for arm Mbed
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/3
 ------------------------------------------- */
#if defined(__MBED__)

#include "mbed.h"
#include "console.h"
#include "emmem.h"
#include <cstdio>
#include <cstdlib>


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

UnbufferedSerial serial(CONSOLE_TX, CONSOLE_RX);

void initialize_console(){
}

em_result read_line(string_t * recycle_buffer) {
    if(recycle_buffer->length == 0){
        recycle_buffer->buffer = (char *)(::std::malloc(sizeof(char) * 256));
        if(recycle_buffer->buffer == nullptr) {
            return EM_RESULT_OUT_OF_MEMORY;
        }
        recycle_buffer->length = 256;
    }
    char * buf = recycle_buffer->buffer;
    int start;
    for(start = 0; start < 254;) {
        int len = serial.read(&(buf[start]), 255-start);
        
        int count = len + start;
        for(int i = start; i < count; ++i) {
            // Normaly, CR(\r) represents to go to the new line.
            if(buf[i] == '\n' || buf[i] == '\r') {
                serial.write(&(buf[start]), i - start);
                start = execute_control_sequence(i-start, start, buf);
                serial.write("\n", sizeof("\n"));
                goto end;
            }
        }
        serial.write(&(buf[start]), len);
        start = execute_control_sequence(len, start, buf);
    }
    end:
    buf[start] = '\0';
    return EM_RESULT_OK;
}

#endif