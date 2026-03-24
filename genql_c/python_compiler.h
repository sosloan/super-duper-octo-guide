/*
 * genql_c/python_compiler.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles COMPUTE intents into Python function definitions.
 *
 * Body key conventions
 * --------------------
 * function   : (string) Name for the generated function.
 *              Defaults to intent->name if absent.
 * params     : (csv)    Ordered parameter names.
 *              Example: kv_set(&body, "params", "x,y")
 * returns    : (string) Return type annotation (optional).
 *              Example: kv_set(&body, "returns", "int")
 * expression : (string) Single Python expression for the function body.
 *              Example: kv_set(&body, "expression", "x * 2")
 * step.N     : (string) One statement line (0-based index N).
 *              Use instead of expression for multi-statement bodies.
 *              Example: kv_set(&body, "step.0", "msg = 'hi'")
 *                       kv_set(&body, "step.1", "return msg")
 *                       kv_set_int(&body, "step_count", 2)
 * step_count : (int)    Number of step.N entries (required if steps used).
 *
 * If neither expression nor step_count is provided the body defaults to
 * a single "pass" statement.
 */
#ifndef GENQL_PYTHON_COMPILER_H
#define GENQL_PYTHON_COMPILER_H

#include "compiler.h"

/* Return a fully configured Compiler struct for Python output. */
Compiler python_compiler_new(void);

#endif /* GENQL_PYTHON_COMPILER_H */
