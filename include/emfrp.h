/** -------------------------------------------
 * @file   emfrp.h
 * @brief  Emfrp Main Functions
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/30
 ------------------------------------------- */
#pragma once
#include <stdbool.h>

typedef struct object_t em_object_t;

// ! EM_EXPORTDECL
#if defined(_WIN32)
#  if defined(EXPORTING)
#    define EM_EXPORTDECL __declspec(dllexport)
#  else
#    define EM_EXPORTDECL __declspec(dllimport)
#  endif
#else // non windows
#  define EM_EXPORTDECL
#endif

typedef em_object_t* (*em_callback)(void);
typedef struct emfrp_t emfrp_t;

emfrp_t * EM_EXPORTDECL emfrp_create(void);
bool EM_EXPORTDECL emfrp_repl(emfrp_t * self, char * str);
bool EM_EXPORTDECL emfrp_add_input_node_definition(emfrp_t * self, char * node_name, em_callback callback);
bool EM_EXPORTDECL emfrp_indicate_node_update(emfrp_t * self, char * node_name, em_object_t * value);
bool EM_EXPORTDECL emfrp_add_output_node_definition(emfrp_t * self, char * node_name, em_callback callback);
