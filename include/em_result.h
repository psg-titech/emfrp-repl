/** -------------------------------------------
 * @file   em_result.h
 * @brief  Status Code
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/27
 ------------------------------------------- */
#pragma once

#define CHKERR(v) if(errres = (v)) goto err;

// ! Result Status
typedef enum em_result_t {
  // ! OK
  EM_RESULT_OK = 0,
  // ! Memory is full.
  EM_RESULT_OUT_OF_MEMORY = 1,
  // ! Invalid Argument
  EM_RESULT_INVALID_ARGUMENT = 2,
  // ! Type Mismatch
  EM_RESULT_TYPE_MISMATCH = 16,
  // ! Missing Identifier
  EM_RESULT_MISSING_IDENTIFIER = 17,
  // ! Unknown Error.
  EM_RESULT_UNKNOWN_ERR
} em_result;
