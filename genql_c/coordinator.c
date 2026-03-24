/*
 * genql_c/coordinator.c
 * ~~~~~~~~~~~~~~~~~~~~~
 * Coordination engine implementation.
 */
#include "coordinator.h"
#include "sql_compiler.h"
#include "python_compiler.h"
#include "schema_compiler.h"
#include "policy_compiler.h"
#include "workflow_compiler.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void coordinator_init(Coordinator *coord) {
    memset(coord, 0, sizeof(*coord));
    absorber_init(&coord->absorber);
}

Coordinator *coordinator_register(Coordinator *coord, Compiler c) {
    if (coord->num_compilers < COORDINATOR_MAX_COMPILERS)
        coord->compilers[coord->num_compilers++] = c;
    return coord;
}

Coordinator *coordinator_use(Coordinator *coord, Middleware mw) {
    absorber_use(&coord->absorber, mw);
    return coord;
}

/* Find the compiler that handles intent->kind; returns NULL if none. */
static const Compiler *find_compiler(const Coordinator *coord,
                                     const Intent *intent) {
    for (int i = 0; i < coord->num_compilers; i++)
        if (compiler_can_compile(&coord->compilers[i], intent))
            return &coord->compilers[i];
    return NULL;
}

CompiledOutput coordinator_compile(Coordinator *coord, const Intent *intent) {
    CompiledOutput out;
    memset(&out, 0, sizeof(out));

    /* Reset global error state before absorbing. */
    genql_last_error[0] = '\0';

    Intent absorbed = absorber_absorb(&coord->absorber, *intent);
    out.intent = absorbed;

    const Compiler *c = find_compiler(coord, &absorbed);
    if (!c) {
        snprintf(genql_last_error, sizeof(genql_last_error),
                 "No compiler registered for intent kind '%s'. "
                 "Register one with coordinator_register().",
                 intent_kind_str(absorbed.kind));
        fprintf(stderr, "ERROR: %s\n", genql_last_error);
        out.code = NULL;
        return out;
    }

    strncpy(out.language, c->target_language, sizeof(out.language) - 1);
    out.code = c->compile(&absorbed);
    return out;
}

CompiledOutput *coordinator_compile_all(Coordinator *coord,
                                        const Intent *intents, int n) {
    CompiledOutput *arr = (CompiledOutput *)malloc(
                              (size_t)n * sizeof(CompiledOutput));
    for (int i = 0; i < n; i++)
        arr[i] = coordinator_compile(coord, &intents[i]);
    return arr;
}

void coordinator_compile_all_free(CompiledOutput *arr, int n) {
    for (int i = 0; i < n; i++)
        compiled_output_free(&arr[i]);
    free(arr);
}

int coordinator_registered_kinds(const Coordinator *coord,
                                 IntentKind *out, int max) {
    int count = 0;
    for (int i = 0; i < coord->num_compilers && count < max; i++) {
        const Compiler *c = &coord->compilers[i];
        for (int k = 0; k < c->num_kinds && count < max; k++)
            out[count++] = c->supported_kinds[k];
    }
    return count;
}

const Role *coordinator_role_for(IntentKind kind) {
    return role_for_kind(kind);
}

const char *coordinator_explain(const Coordinator *coord, const Intent *intent,
                                char *buf, int buflen) {
    const Role     *role = role_for_kind(intent->kind);
    const Compiler *c    = find_compiler(coord, intent);
    const char     *cname = c ? c->target_language : "(unregistered)";

    if (role) {
        snprintf(buf, buflen,
                 "Intent '%s' [%s]\n"
                 "  Role:     %s — %s\n"
                 "  Compiler: %s compiler\n"
                 "  Language: %s",
                 intent->name, intent_kind_str(intent->kind),
                 role->name, role->description,
                 cname, role->target_language);
    } else {
        snprintf(buf, buflen,
                 "Intent '%s' [%s] — no role defined",
                 intent->name, intent_kind_str(intent->kind));
    }
    return buf;
}

Coordinator coordinator_default(void) {
    Coordinator coord;
    coordinator_init(&coord);

    absorber_use(&coord.absorber, mw_normalise_name);
    absorber_use(&coord.absorber, mw_default_operation);
    absorber_use(&coord.absorber, mw_require_table);

    coordinator_register(&coord, sql_compiler_new());
    coordinator_register(&coord, python_compiler_new());
    coordinator_register(&coord, schema_compiler_new());
    coordinator_register(&coord, policy_compiler_new());
    coordinator_register(&coord, workflow_compiler_new());

    return coord;
}
