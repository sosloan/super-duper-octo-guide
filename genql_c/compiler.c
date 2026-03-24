/*
 * genql_c/compiler.c
 * ~~~~~~~~~~~~~~~~~~
 * Compiler helper implementations.
 */
#include "compiler.h"
#include <stdlib.h>

void compiled_output_free(CompiledOutput *out) {
    if (out) {
        free(out->code);
        out->code = NULL;
    }
}

int compiler_can_compile(const Compiler *c, const Intent *intent) {
    for (int i = 0; i < c->num_kinds; i++)
        if (c->supported_kinds[i] == intent->kind) return 1;
    return 0;
}
