/** -------------------------------------------
 * @file   emmem.h
 * @brief  Memory Management Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */

#pragma once

#include <stdlib.h>
#include <string.h>
#include "misc.h"

//#if __STD_VERSION__ <= 201710L
#define em_strdup(s) strdup(s)
#define em_malloc(size) malloc(size)
#define em_realloc(ptr, size) realloc(ptr, size)
#define em_free(ptr) free(ptr)
#if _MSC_VER || __APPLE__
#define em_allocarray(nmemb, size) malloc((size) * (nmemb))
#define em_reallocarray(ptr, size, nmemb) realloc(ptr, (size) * (nmemb))
#else
#define em_allocarray(nmemb, size) reallocarray(nullptr, nmemb, size)
#define em_reallocarray(ptr, nmemb, size) reallocarray(ptr,  nmemb, size)
#endif
// I cannot figure out the equivalent expression before C23.
//#else
//#define em_malloc(elem) (typeof(elem)*)malloc(sizeof(typeof(elem)))
//#endif
