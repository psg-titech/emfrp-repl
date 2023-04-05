/** -------------------------------------------
 * @file   em_result.h
 * @brief  Status Code
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/3/20
 ------------------------------------------- */
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

// Apple Clang warns, if there is not additional parences.
#define CHKERR(v)                                                                                  \
  {                                                                                                \
    if((errres = (v))) goto err;                                                                   \
  }
#define CHKERR2(label, v)                                                                          \
  {                                                                                                \
    if((errres = (v))) goto label;                                                                 \
  }

// Execute test, and goto err if the result is false.
#define TEST_AND_ERROR(test, reason)                                                               \
  if(test) {                                                                                       \
    errres = (reason);                                                                             \
    goto err;                                                                                      \
  }

  // ! Result Status
  typedef enum em_result_t
  {
    // ! OK
    EM_RESULT_OK = 0,
    // ! Unknown Error.
    EM_RESULT_UNKNOWN_ERR = 1,
    // ! Memory is full.
    EM_RESULT_OUT_OF_MEMORY = 2,
    // ! Out Of Index
    EM_RESULT_OUT_OF_INDEX = 3,
    // ! Invalid Argument
    EM_RESULT_INVALID_ARGUMENT = 4,
    // ! memory_manager_t::worklist is overflowed.
    EM_RESULT_GC_WORKLIST_OVERFLOW = 5,
    // ! Stack is overflowed.
    EM_RESULT_STACK_OVERFLOW = 6,
    // Below, users are responsible.
    // ! Type Mismatch
    EM_RESULT_TYPE_MISMATCH = 16,
    // ! Missing Identifier
    EM_RESULT_MISSING_IDENTIFIER = 17,
    // ! Cyclic Reference
    EM_RESULT_CYCLIC_REFERENCE = 18,
    // ! Parse Error
    EM_RESULT_PARSE_ERROR = 19
  } em_result;

  extern const char * const EM_RESULT_STR_TABLE[];

#ifdef __cplusplus
}
#endif /* __cplusplus */
