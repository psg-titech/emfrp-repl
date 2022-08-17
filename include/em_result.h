/** -------------------------------------------
 * @file   em_result.h
 * @brief  Status Code
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/8/17
 ------------------------------------------- */
#pragma once

// ! Result Status
typedef enum em_result_t {
  // ! OK
  EM_RESULT_OK = 0,
  // ! Memory is full.
  EM_RESULT_OUT_OF_MEMORY = 1,
  // ! Invalid Argument
  EM_RESULT_INVALID_ARGUMENT = 2,
  // ! Unknown Error.
  EM_RESULT_UNKNOWN_ERR
} em_result;
