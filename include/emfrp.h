/** -------------------------------------------
 * @file   emfrp.h
 * @brief  Emfrp Main Functions
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/28
 ------------------------------------------- */
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct object_t em_object_t;

// ! EM_EXPORTDECL
#if defined(_WIN32)
#  if defined(EMFRP_EXPORTING)
#    define EM_EXPORTDECL __declspec(dllexport)
#  else
#    define EM_EXPORTDECL __declspec(dllimport)
#  endif
#else // non windows
#  define EM_EXPORTDECL
#endif

typedef em_object_t * (*em_input_node_callback)(void);
typedef void (*em_output_node_callback)(em_object_t *);
typedef struct emfrp_t emfrp_t;
#if __cplusplus
extern "C" {
#endif
EM_EXPORTDECL emfrp_t * emfrp_create(void);
  EM_EXPORTDECL bool emfrp_repl(emfrp_t * self, char * str, em_object_t ** value);
EM_EXPORTDECL bool emfrp_add_input_node_definition(emfrp_t * self, char * node_name, em_input_node_callback callback);
EM_EXPORTDECL bool emfrp_indicate_node_update(emfrp_t * self, char * node_name, em_object_t * value);
EM_EXPORTDECL bool emfrp_update(emfrp_t * self);
EM_EXPORTDECL bool emfrp_add_output_node_definition(emfrp_t * self, char * node_name, em_output_node_callback callback);
EM_EXPORTDECL em_object_t * emfrp_create_int_object(int32_t num);
EM_EXPORTDECL em_object_t * emfrp_get_true_object(void);
EM_EXPORTDECL em_object_t * emfrp_get_false_object(void);
EM_EXPORTDECL int32_t emfrp_get_integer(em_object_t * v);
#if __cplusplus
}
#endif
