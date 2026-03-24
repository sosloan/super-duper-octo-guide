/*
 * genql_c/sql_compiler.h
 * ~~~~~~~~~~~~~~~~~~~~~~
 * Compiles DATA intents into SQL statements.
 *
 * Body key conventions
 * --------------------
 * table      : (string) Target table name.
 * operation  : (string) "select" | "insert" | "update" | "delete".
 *              Defaults to "select" when mw_default_operation is used.
 * col        : (csv) Columns to retrieve for SELECT; omit or "*" for all.
 *              Example: kv_set(&body, "col", "id,name")
 * where.COL  : (string) Equality condition value for column COL.
 *              Example: kv_set(&body, "where.id", "5")
 *                       kv_set(&body, "where.name", "alice")
 * value.COL  : (string) Column value for INSERT / UPDATE.
 *              Example: kv_set(&body, "value.name", "bob")
 *                       kv_set(&body, "value.age",  "30")
 *
 * Numeric strings (digits only, optionally leading '-') are emitted
 * without quotes; all other strings are single-quoted with ' escaped as ''.
 */
#ifndef GENQL_SQL_COMPILER_H
#define GENQL_SQL_COMPILER_H

#include "compiler.h"

/* Return a fully configured Compiler struct for the SQL language. */
Compiler sql_compiler_new(void);

#endif /* GENQL_SQL_COMPILER_H */
