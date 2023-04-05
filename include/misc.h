/** -------------------------------------------
 * @file   misc.h
 * @brief  Miscellaneous
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/11
 ------------------------------------------- */
#pragma once
#if defined(_WIN32)
#include <Windows.h>
#include <tchar.h>
#endif
#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
#if defined(_WIN32)
  typedef TCHAR char_t;
#define em_strlen(s) _tcslen(s)
#define strdup(s)    _strdup(s)
#else
typedef char char_t;
#define em_strlen(s) strlen(s)
#endif

#if defined(__cplusplus)
#if __cplusplus < 201100L
// nullptr is appeared in C23.
#define nullptr NULL
  typedef void * nullptr_t;
#endif
#else
#if __STD_VERSION__ <= 201710L
// nullptr is appeared in C23.
#define nullptr NULL
typedef void * nullptr_t;
#endif
#endif

#if !defined(MIN)
#define MIN(v1, v2) (v1) > (v2) ? (v2) : (v1)
#endif
#if !defined(MAX)
#define MAX(v1, v2) (v1) < (v2) ? (v2) : (v1)
#endif

#if _MSC_VER
#define DEBUGBREAK __debugbreak()
#elif __GNUC__
#define DEBUGBREAK __builtin_trap()
#elif __clang__
#define DEBUGBREAK __builtin_debugtrap()
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
