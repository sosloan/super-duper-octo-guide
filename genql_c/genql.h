/*
 * genql_c/genql.h
 * ~~~~~~~~~~~~~~~
 * Public umbrella header — include this single header to use GenQL from C.
 *
 * GenQL — An Executable Theory of Coordination (C implementation)
 * ---------------------------------------------------------------
 * GenQL is a coordination framework for systems that must hold more than
 * one language at once.  Instead of forcing every concern through a single
 * notation, GenQL lets you declare intent in a neutral, typed form and then
 * routes each piece of intent to the compiler that knows how to express or
 * enforce it correctly.
 *
 * Quick start
 * -----------
 *
 *   #include "genql.h"
 *
 *   // Create a default coordinator (all five compilers + absorber chain).
 *   Coordinator coord = coordinator_default();
 *
 *   // DATA -> SQL
 *   KV body = {0};
 *   kv_set(&body, "table", "users");
 *   Intent intent = intent_new(GENQL_KIND_DATA, "get_users", &body);
 *   CompiledOutput out = coordinator_compile(&coord, &intent);
 *   printf("%s\n", out.code);   // SELECT * FROM "users";
 *   compiled_output_free(&out);
 *
 *   // COMPUTE -> Python
 *   KV body2 = {0};
 *   kv_set(&body2, "function",   "double");
 *   kv_set(&body2, "params",     "x");
 *   kv_set(&body2, "expression", "x * 2");
 *   Intent i2 = intent_new(GENQL_KIND_COMPUTE, "double", &body2);
 *   CompiledOutput out2 = coordinator_compile(&coord, &i2);
 *   printf("%s\n", out2.code);
 *   compiled_output_free(&out2);
 *
 * See the individual compiler headers for full body key documentation.
 */
#ifndef GENQL_H
#define GENQL_H

#include "kv.h"
#include "strbuf.h"
#include "intent.h"
#include "roles.h"
#include "absorber.h"
#include "compiler.h"
#include "sql_compiler.h"
#include "python_compiler.h"
#include "schema_compiler.h"
#include "policy_compiler.h"
#include "workflow_compiler.h"
#include "coordinator.h"

#endif /* GENQL_H */
