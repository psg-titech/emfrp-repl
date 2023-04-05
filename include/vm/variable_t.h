/** -------------------------------------------
 * @file   variable_t.h
 * @brief  The variable table.
 * @author Go Suzuki <puyogo.suzuki@gmail.com>
 * @date   2023/1/11
 ------------------------------------------- */
#pragma once
#include "string_t.h"
#include "collections/dictionary_t.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

  struct object_t;
  struct dictionary_t;
  struct machine_t;

  // ! Variable Definition.
  typedef struct variable_t
  {
    // ! Name of the variable.
    string_t name;
    // ! Value of the variable.
    struct object_t * value;
  } variable_t;

  // ! The Variable Table.
  typedef struct variable_table_t
  {
    // ! Parent of the variable.
    struct variable_table_t * parent;
    // ! Object Reference to this. (for closure)
    struct object_t * this_object_ref;
    // ! Table.
    dictionary_t /*<variable_t>*/ table;
  } variable_table_t;

  // ! Construct variable_t.
  /* !
 * \param out The result
 * \param name The name
 * \return The status code
 */
  static inline em_result
  variable_new(variable_t * out, string_t name)
  {
    out->name  = name;
    out->value = nullptr;
    return EM_RESULT_OK;
  }
  // ! Construct variable_table_t.
  /* !
 * \param m The machine
 * \param out The result
 * \param parent The parent
 * \return The status code
 */
  em_result
  variable_table_new(struct machine_t * m, variable_table_t * out, variable_table_t * parent);

  // ! Assign given value to the variable.
  /* !
 * \param m The machine
 * \param self The table to search or added.
 * \param name The name of the variable to be assigned to.
 * \param value The value to be assigned.
 * \return The status code
 */
  em_result variable_table_assign(
    struct machine_t * m, variable_table_t * self, string_t * name, struct object_t * value);

  // ! Lookup from the variable table.
  /* !
 * \param self The table to search.
 * \param out The result.
 * \param name The name of the variable to be searched.
 * \return Wether it is found.
 */
  bool variable_table_lookup(variable_table_t * self, struct object_t ** out, string_t * name);

  // ! Freeing Deeply variable_t
  /* !
 * \param v The variable to be freed
 */
  void variable_deep_free(variable_t * v);

  // ! Freeing variable_table_t
  /* !
 * \param v The variable table to be freed
 */
  void variable_table_free(variable_table_t * v);

#ifdef __cplusplus
}
#endif /* __cplusplus */
