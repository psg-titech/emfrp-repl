/** -------------------------------------------
 * @file   machine.h
 * @brief  Emfrp REPL Machine
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/26
 ------------------------------------------- */

#pragma once
#include "em_result.h"

typedef struct machine_t {

} machine_t;

// ! Constructor of machine_t.
/* !
 * \param out The result.
 * \return The status code.
 */
em_result
machine_new(machine_t * out);
