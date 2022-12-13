/** -------------------------------------------
 * @file   journal_t.h
 * @brief  Journaling on redefining nodes.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2022/12/12
 ------------------------------------------- */
#pragma once
#include "collections/list_t.h"
#include "vm/node_t.h"

typedef struct journal_t {
  // The next entry of the journal.
  struct journal_t * next;
  // Where journal_t::what is placed.
  node_t ** where_placed;
  // Which the node.
  node_t * what;
} journal_t;

// ! The default value of journal_t *.
#define journal_defualt() nullptr

// ! Remove the pointer to the node(node) from the execution list.
/* !
 * \param exec_sequences The list of execution list.
 * \param out The journal
 * \param node The node which remove from.
 */
em_result
remove_defined_node(list_t /*<exec_sequence_t>*/ * exec_sequences, journal_t ** out, node_t * node);

// ! Revert by journal_t.
/* !
 * \param j The journal which records the modifications.
 */
void revert_from_journal(journal_t * j);

// ! Freeing journal_t.
/* !
 * \param j The journal to be freed.
 */
void journal_free(journal_t ** j);
