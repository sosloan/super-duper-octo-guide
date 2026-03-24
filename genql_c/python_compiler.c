/*
 * genql_c/python_compiler.c
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles COMPUTE intents into Python function definitions.
 */
#include "python_compiler.h"
#include "strbuf.h"
#include "kv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char *python_compile(const Intent *intent) {
    StrBuf sb; sb_init(&sb);
    const KV *body = &intent->body;

    const char *fn_name    = kv_get(body, "function");
    const char *params_str = kv_get(body, "params");
    const char *returns    = kv_get(body, "returns");
    const char *expression = kv_get(body, "expression");
    int step_count         = kv_get_int(body, "step_count", 0);

    if (fn_name[0] == '\0') fn_name = intent->name;

    /* ---- Function signature ---- */
    sb_appendf(&sb, "def %s(", fn_name);
    if (params_str[0] != '\0') {
        char params[32][KV_VAL_MAX];
        int np = kv_split_csv(params_str, params, 32);
        for (int i = 0; i < np; i++) {
            if (i > 0) sb_append(&sb, ", ");
            sb_append(&sb, params[i]);
        }
    }
    sb_append(&sb, ")");
    if (returns[0] != '\0') sb_appendf(&sb, " -> %s", returns);
    sb_append(&sb, ":");

    /* ---- Function body ---- */
    if (step_count > 0) {
        for (int i = 0; i < step_count; i++) {
            char key[64];
            snprintf(key, sizeof(key), "step.%d", i);
            const char *step = kv_get(body, key);
            sb_appendf(&sb, "\n    %s", step);
        }
    } else if (expression[0] != '\0') {
        sb_appendf(&sb, "\n    return %s", expression);
    } else {
        sb_append(&sb, "\n    pass");
    }

    return sb_done(&sb);
}

Compiler python_compiler_new(void) {
    Compiler c;
    memset(&c, 0, sizeof(c));
    c.supported_kinds[0] = GENQL_KIND_COMPUTE;
    c.num_kinds = 1;
    strncpy(c.target_language, "python", sizeof(c.target_language) - 1);
    c.compile = python_compile;
    return c;
}
