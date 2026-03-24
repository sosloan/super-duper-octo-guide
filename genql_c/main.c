/*
 * genql_c/main.c
 * ~~~~~~~~~~~~~~
 * Demo program — mirrors the Python "Quick start" from README.md but in C.
 *
 * Build: make genql_demo   (or: make all)
 * Run:   ./genql_demo
 */
#include "genql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_separator(void) {
    puts("--------------------------------------------------");
}

int main(void) {
    /* ------------------------------------------------------------------ */
    /* 1. Create the default coordinator.                                   */
    /* ------------------------------------------------------------------ */
    Coordinator coord = coordinator_default();

    puts("GenQL C Demo");
    print_separator();

    /* ------------------------------------------------------------------ */
    /* 2. DATA -> SQL (SELECT *)                                            */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        Intent intent = intent_new(GENQL_KIND_DATA, "get_users", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n  %s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 3. DATA -> SQL (SELECT columns with WHERE)                          */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "table",     "users");
        kv_set(&body, "operation", "select");
        kv_set(&body, "col",       "id,name");
        kv_set(&body, "where.id",  "42");
        Intent intent = intent_new(GENQL_KIND_DATA, "find_user", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n  %s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 4. DATA -> SQL (INSERT)                                             */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "table",        "users");
        kv_set(&body, "operation",    "insert");
        kv_set(&body, "value.name",   "alice");
        kv_set(&body, "value.email",  "alice@example.com");
        Intent intent = intent_new(GENQL_KIND_DATA, "add_user", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n  %s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 5. COMPUTE -> Python (expression)                                   */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "function",   "double");
        kv_set(&body, "params",     "x");
        kv_set(&body, "expression", "x * 2");
        Intent intent = intent_new(GENQL_KIND_COMPUTE, "double", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n  %s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 6. COMPUTE -> Python (multi-step)                                   */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "function", "greet");
        kv_set(&body, "params",   "name");
        kv_set(&body, "step.0",   "msg = f\"Hello, {name}!\"");
        kv_set(&body, "step.1",   "return msg");
        kv_set_int(&body, "step_count", 2);
        Intent intent = intent_new(GENQL_KIND_COMPUTE, "greet", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n  %s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 7. SCHEMA -> JSON Schema                                            */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "title",             "User");
        kv_set(&body, "prop_names",        "id,name");
        kv_set(&body, "prop.id.type",      "integer");
        kv_set(&body, "prop.name.type",    "string");
        kv_set(&body, "required",          "id,name");
        Intent intent = intent_new(GENQL_KIND_SCHEMA, "UserSchema", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n%s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 8. POLICY -> Policy DSL (allow)                                     */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "subject",  "admin");
        kv_set(&body, "action",   "read");
        kv_set(&body, "resource", "document");
        kv_set(&body, "effect",   "allow");
        Intent intent = intent_new(GENQL_KIND_POLICY, "admin_read", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n%s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 9. POLICY -> Policy DSL (with conditions)                           */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "subject",    "user");
        kv_set(&body, "action",     "write");
        kv_set(&body, "resource",   "file");
        kv_set(&body, "effect",     "allow");
        kv_set(&body, "cond.0",     "user.id == file.owner_id");
        kv_set_int(&body, "cond_count", 1);
        Intent intent = intent_new(GENQL_KIND_POLICY, "owner_only", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n%s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 10. WORKFLOW -> Workflow DSL                                         */
    /* ------------------------------------------------------------------ */
    {
        KV body = {0};
        kv_set(&body, "step.0.name",       "validate");
        kv_set(&body, "step.0.role",       "schema");
        kv_set(&body, "step.0.action",     "check_input");
        kv_set(&body, "step.1.name",       "persist");
        kv_set(&body, "step.1.role",       "data");
        kv_set(&body, "step.1.action",     "insert_user");
        kv_set(&body, "step.1.depends_on", "validate");
        kv_set_int(&body, "step_count", 2);
        Intent intent = intent_new(GENQL_KIND_WORKFLOW, "onboard_user", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("[%s]  %s\n%s\n", out.language, out.intent.name, out.code);
        compiled_output_free(&out);
    }

    /* ------------------------------------------------------------------ */
    /* 11. explain() — coordinator introspection                           */
    /* ------------------------------------------------------------------ */
    print_separator();
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        Intent intent = intent_new(GENQL_KIND_DATA, "get_users", &body);
        char explanation[512];
        coordinator_explain(&coord, &intent, explanation, sizeof(explanation));
        printf("explain():\n%s\n", explanation);
    }

    /* ------------------------------------------------------------------ */
    /* 12. Name normalisation middleware demo                              */
    /* ------------------------------------------------------------------ */
    print_separator();
    {
        KV body = {0};
        kv_set(&body, "table", "orders");
        Intent intent = intent_new(GENQL_KIND_DATA, "  GET_ORDERS  ", &body);
        CompiledOutput out = coordinator_compile(&coord, &intent);
        printf("name normalised: '%s'\n", out.intent.name);
        compiled_output_free(&out);
    }

    puts("Done.");
    return 0;
}
