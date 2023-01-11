/** -------------------------------------------
 * @file   program.h
 * @brief  Emfrp Program Representation
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/11
 ------------------------------------------- */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct object_t;

typedef struct object_t * (*exec_callback_t)(void);
typedef em_result (*foreign_func_t)(struct object_t **, struct object_t *);
#define EXEC_SEQUENCE_PROGRAM_KIND_SHIFT 3

// ! Program kind of node.
/* !
 * LSB represents `phantom`, which is created before verification.
 * LSB + 1 represents `modified`, whose defined_nodes are written into the journal.
 * LSB + 2 represents `last failed`, which failed at the last time.
 */
typedef enum emfrp_program_kind {
  // ! no program.(it may be updated via emfrp_indicate_node_update.)
  EMFRP_PROGRAM_KIND_NOTHING = 1 << EXEC_SEQUENCE_PROGRAM_KIND_SHIFT,
  // ! containing AST.
  EMFRP_PROGRAM_KIND_AST = 2 << EXEC_SEQUENCE_PROGRAM_KIND_SHIFT,
  // ! containing callback.
  EMFRP_PROGRAM_KIND_CALLBACK = 3 << EXEC_SEQUENCE_PROGRAM_KIND_SHIFT,
  // ! containing record constructor.
  EMFRP_PROGRAM_KIND_RECORD_CONSTRUCT = 4 << EXEC_SEQUENCE_PROGRAM_KIND_SHIFT,
  // ! containing record accessor.
  EMFRP_PROGRAM_KIND_RECORD_ACCESS = 5 << EXEC_SEQUENCE_PROGRAM_KIND_SHIFT
} emfrp_program_kind;


#ifdef __cplusplus
}
#endif /* __cplusplus */
