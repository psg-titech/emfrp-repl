/** -------------------------------------------
 * @file   emmem.h
 * @brief  Memory Management Implementation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/11
 ------------------------------------------- */

#pragma once

#include <stdlib.h>
#include <string.h>
#include "misc.h"
#include "em_result.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define em_strdup(s) strdup(s)
static inline em_result
em_malloc(void ** out, size_t size) {
  void * t = malloc(size);
  if(t == nullptr)
    return EM_RESULT_OUT_OF_MEMORY;
  *out = t;
  return EM_RESULT_OK;
}

static inline em_result
em_realloc(void ** out, void * ptr, size_t size) {
  void * t = realloc(ptr, size);
  if(t == nullptr)
    return EM_RESULT_OUT_OF_MEMORY;
  *out = t;
  return EM_RESULT_OK;
}

#define em_free(ptr) free(ptr)
#if _MSC_VER || __APPLE__
#define em_allocarray(o, nmemb, size) em_malloc(o, (size) * (nmemb))
#define em_reallocarray(o, ptr, size, nmemb) em_realloc(o, ptr, (size) * (nmemb))
#else
static inline em_result
em_reallocarray(void ** out, void * ptr, size_t nmemb, size_t size) {
  void * t = reallocarray(ptr, nmemb, size);
  if(t == nullptr)
    return EM_RESULT_OUT_OF_MEMORY;
  *out = t;
  return EM_RESULT_OK;
}
#define em_allocarray(o, nm, s) em_reallocarray(o, nullptr, nm, s)
#endif
// I cannot figure out the equivalent expression before C23.
//#define em_malloc(elem) (typeof(elem)*)malloc(sizeof(typeof(elem)))

#ifdef __cplusplus
}
#endif /* __cplusplus */
