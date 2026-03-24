/*
 * genql_c/schema_compiler.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles SCHEMA intents into JSON Schema (draft 2020-12) strings.
 *
 * Body key conventions
 * --------------------
 * title          : (string) Human-readable schema title.
 *                  Defaults to intent->name if absent.
 * description    : (string) Optional schema description.
 * prop_names     : (csv)    Ordered list of property names.
 *                  Example: kv_set(&body, "prop_names", "id,name")
 * prop.NAME.FIELD: (string) A field of the property named NAME.
 *                  Common fields: type, description, enum, default.
 *                  Example: kv_set(&body, "prop.id.type",   "integer")
 *                           kv_set(&body, "prop.name.type", "string")
 * required       : (csv)    Names of required properties.
 *                  Example: kv_set(&body, "required", "id,name")
 * add_props      : (string) "true" to allow additional properties.
 *                  Defaults to "false".
 */
#ifndef GENQL_SCHEMA_COMPILER_H
#define GENQL_SCHEMA_COMPILER_H

#include "compiler.h"

/* Return a fully configured Compiler struct for JSON Schema output. */
Compiler schema_compiler_new(void);

#endif /* GENQL_SCHEMA_COMPILER_H */
