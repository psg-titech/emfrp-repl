#include <stdio.h>
#include "hal/console.h"

int main(void) {
  string_t line;
  string_null(&line);
  read_line(&line);
  return 0;
}
