/** -------------------------------------------
 * @file   misc.h
 * @brief  Miscellaneous
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */
#pragma once
#if defined(_WIN32)
#include <Windows.h>
#include <tchar.h>
typedef TCHAR char_t;
#define em_strlen(s) _tcslen(s)
#define strdup(s) _strdup(s)
#else
typedef char char_t;
#define em_strlen(s) strlen(s)
#endif

#if __STD_VERSION__ <= 201710L
// nullptr is appeared in C23.
#define nullptr NULL
#endif
