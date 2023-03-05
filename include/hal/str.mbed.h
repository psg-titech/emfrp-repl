/** -------------------------------------------
 * @file   str.mbed.h
 * @brief  str pollyfill functions for mbed
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/3
 ------------------------------------------- */
#if defined(__MBED__)
#pragma once
#if _cplusplus
extern "C" {
#endif
#include "stdio.h"
#define strnlen(str, noe) strlen(str)

#if _cplusplus
}
#endif
#endif