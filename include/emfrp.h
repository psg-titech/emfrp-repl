/** -------------------------------------------
 * @file   emfrp.h
 * @brief  Emfrp Main Functions
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/10/12
 ------------------------------------------- */
#pragma once
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

typedef em_object_t* (*em_callback)(void);
typedef struct emfrp_t emfrp_t;

EM_EXPORTDECL emfrp_t * emfrp_create(void);
EM_EXPORTDECL bool emfrp_repl(emfrp_t * self, char * str);
EM_EXPORTDECL bool emfrp_add_input_node_definition(emfrp_t * self, char * node_name, em_callback callback);
EM_EXPORTDECL bool emfrp_indicate_node_update(emfrp_t * self, char * node_name, em_object_t * value);
EM_EXPORTDECL bool emfrp_add_output_node_definition(emfrp_t * self, char * node_name, em_callback callback);
