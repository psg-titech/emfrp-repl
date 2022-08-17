/** -------------------------------------------
 * @file   console.h
 * @brief  Console Abstruction
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */

#pragma once
#include "em_result.h"
#include "string_t.h"

// ! Initialize Console
void initialize_console(void);
// ! Read line.
/* !
 * \param recycle_buffer the Buffer, the content is erased and reused.
 * \return Result.
 */
em_result read_line(string_t * recycle_buffer);
