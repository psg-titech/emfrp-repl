/** -------------------------------------------
 * @file   emfrp.h
 * @brief  Emfrp Main Functions
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/8/28
 ------------------------------------------- */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "em_result.h"

#if __cplusplus
extern "C"
{
#endif

  typedef struct object_t em_object_t;

// ! EM_EXPORTDECL
#if defined(_WIN32)
#if defined(EMFRP_EXPORTING)
#define EM_EXPORTDECL __declspec(dllexport)
#else
#define EM_EXPORTDECL __declspec(dllimport)
#endif
#else  // non windows
#define EM_EXPORTDECL
#endif

  typedef em_object_t * (*em_input_callback)(void);
  typedef void (*em_output_callback)(em_object_t *);
  typedef struct emfrp_t emfrp_t;

  EM_EXPORTDECL em_result emfrp_create(emfrp_t ** result);
  EM_EXPORTDECL em_result emfrp_repl(emfrp_t * self, const char * str, em_object_t ** value);
  EM_EXPORTDECL em_result
  emfrp_add_input_node(emfrp_t * self, char * node_name, em_input_callback callback);
  EM_EXPORTDECL em_result
  emfrp_add_output_node(emfrp_t * self, char * node_name, em_output_callback callback);
  EM_EXPORTDECL em_result
  emfrp_set_node_value(emfrp_t * self, char * node_name, em_object_t * value);
  EM_EXPORTDECL em_result     emfrp_update(emfrp_t * self);
  EM_EXPORTDECL em_object_t * emfrp_create_int_object(int32_t num);
  EM_EXPORTDECL em_object_t * emfrp_get_true_object(void);
  EM_EXPORTDECL em_object_t * emfrp_get_false_object(void);
  EM_EXPORTDECL int32_t       emfrp_get_integer(em_object_t * v);

  EM_EXPORTDECL void emfrp_print_object(em_object_t * v);
#if __cplusplus
}
#endif /* __cplusplus */
