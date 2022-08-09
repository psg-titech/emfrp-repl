#if defined(unix)
#include "hal/console.h"
#include "string_t.h"
#include <editline/readline.h>

void
initialize_console(void) {
  rl_readline_name = "emfrp-repl";
}

em_result
read_line(string_t * recycle_buffer) {
  char * line;
  string_free(recycle_buffer);
  line = readline("emfrp: ");
  if (line == nullptr)
    return EM_RESULT_UNKNOWN_ERR;
  string_new1(recycle_buffer, line);
  return EM_RESULT_OK;
}

#endif

