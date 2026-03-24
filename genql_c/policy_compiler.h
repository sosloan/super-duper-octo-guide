/*
 * genql_c/policy_compiler.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles POLICY intents into a declarative policy DSL.
 *
 * Body key conventions
 * --------------------
 * subject    : (string) Actor the rule applies to.  Default: "any".
 * action     : (string) Operation governed.         Default: "any".
 * resource   : (string) Resource being protected.   Default: "any".
 * effect     : (string) "allow" or "deny".          Default: "deny".
 * priority   : (int)    Rule priority (higher wins). Default: 0.
 * cond.N     : (string) A condition predicate (0-based index N).
 * cond_count : (int)    Number of cond.N entries.
 *
 * Example
 * -------
 *   kv_set(&body, "subject",    "admin");
 *   kv_set(&body, "action",     "read");
 *   kv_set(&body, "resource",   "document");
 *   kv_set(&body, "effect",     "allow");
 *   kv_set(&body, "cond.0",     "user.active == true");
 *   kv_set_int(&body, "cond_count", 1);
 */
#ifndef GENQL_POLICY_COMPILER_H
#define GENQL_POLICY_COMPILER_H

#include "compiler.h"

/* Return a fully configured Compiler struct for the policy DSL. */
Compiler policy_compiler_new(void);

#endif /* GENQL_POLICY_COMPILER_H */
