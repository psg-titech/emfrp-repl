#pragma once
#include "em_result.h"
#include "string_t.h"
#include <editline/readline.h>

void initialize_console(void);
em_result read_line(string_t * recycle_buffer);
