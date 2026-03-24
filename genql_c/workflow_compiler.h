/*
 * genql_c/workflow_compiler.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles WORKFLOW intents into a step-sequence workflow DSL.
 *
 * Body key conventions
 * --------------------
 * step_count       : (int)    Number of steps.
 * step.N.name      : (string) Step identifier (0-based N).
 * step.N.role      : (string) Role responsible for the step.
 * step.N.action    : (string) What the step does.
 * step.N.on_failure: (string) Step-level failure policy (optional).
 *                   Defaults to the workflow-level on_failure.
 * step.N.depends_on: (csv)    Names of prerequisite steps (optional).
 * on_failure       : (string) Workflow-level default failure handling.
 *                   Default: "abort".
 *
 * Example
 * -------
 *   kv_set(&body, "step.0.name",   "validate");
 *   kv_set(&body, "step.0.role",   "schema");
 *   kv_set(&body, "step.0.action", "check_input");
 *   kv_set(&body, "step.1.name",   "persist");
 *   kv_set(&body, "step.1.role",   "data");
 *   kv_set(&body, "step.1.action", "insert_user");
 *   kv_set(&body, "step.1.depends_on", "validate");
 *   kv_set_int(&body, "step_count", 2);
 */
#ifndef GENQL_WORKFLOW_COMPILER_H
#define GENQL_WORKFLOW_COMPILER_H

#include "compiler.h"

/* Return a fully configured Compiler struct for the workflow DSL. */
Compiler workflow_compiler_new(void);

#endif /* GENQL_WORKFLOW_COMPILER_H */
