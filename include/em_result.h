/** -------------------------------------------
 * @file   em_result.h
 * @brief  Status Code
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/14
 ------------------------------------------- */
#pragma once

// Apple Clang warns, if there is not additional parences.
#define CHKERR(v) if((errres = (v))) goto err;

// ! Result Status
typedef enum em_result_t {
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
  // Below, users are responsible.
  // ! Type Mismatch
  EM_RESULT_TYPE_MISMATCH = 16,
  // ! Missing Identifier
  EM_RESULT_MISSING_IDENTIFIER = 17,
  // ! Cyclic Reference
  EM_RESULT_CYCLIC_REFERENCE = 18
} em_result;

static const char * EM_RESULT_STR_TABLE[] = {
  "Ok",
  "Unknwon Error",
  "Out of Memory",
  "Out of Index",
  "Invalid Argument",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "Type Mismatch",
  "Missing Identifier",
  "Cyclic Referencing",
};
