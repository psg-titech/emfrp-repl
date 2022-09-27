/** -------------------------------------------
 * @file   node_t.h
 * @brief  Emfrp Node Definition(IR)
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/9/27
 ------------------------------------------- */
#pragma once
#include "ast.h"
#include "em_result.h"

// ! Program kind of node.
typedef enum node_program_kind {
  // ! Peripheral node.
  NODE_PROGRAM_KIND_PERIPHERAL,
  // ! containing AST.
  NODE_PROGRAM_KIND_AST,
} node_program_kind;

// ! Node definition struct.
typedef struct node_t {
  // ! Name of node.
  string_t name;
  // ! Kind of node_t::program.
  node_program_kind program_kind;
  // ! Program
  union {
    // ! AST
    parser_expression_t * ast;
  } program;
} node_t;

// ! Construct node_t from AST.
/* !
 * \param out The result
 * \param name The name
 * \param ast The program
 */
em_result node_new_ast(node_t * out, string_t name, parser_expression_t * ast); 
