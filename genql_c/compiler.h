/*
 * genql_c/compiler.h
 * ~~~~~~~~~~~~~~~~~~
 * Abstract compiler type and compiled-output container.
 *
 * A Compiler is responsible for transforming one or more IntentKind values
 * into concrete code in its target language.  The compile function pointer
 * returns a heap-allocated string; the caller must free() it.
 */
#ifndef GENQL_COMPILER_H
#define GENQL_COMPILER_H

#include "intent.h"

/* compile() returns a heap-allocated NUL-terminated string.
 * The caller is responsible for calling free() on the result.
 * Returns NULL on error. */
typedef char *(*CompileFn)(const Intent *);

#define COMPILER_MAX_KINDS 8

/*
 * A registered language compiler.
 *
 * Fields
 * ------
 * supported_kinds  : Intent kinds this compiler handles.
 * num_kinds        : Number of valid entries in supported_kinds.
 * target_language  : Identifier of the emitted language (e.g. "sql").
 * compile          : Function pointer to the compilation routine.
 */
typedef struct {
    IntentKind supported_kinds[COMPILER_MAX_KINDS];
    int        num_kinds;
    char       target_language[64];
    CompileFn  compile;
} Compiler;

/*
 * The result of compiling a single Intent.
 *
 * Fields
 * ------
 * intent   : The (post-absorber) intent that was compiled.
 * language : Target language identifier (matches compiler.target_language).
 * code     : Compiled source / expression.  Heap-allocated; call
 *            compiled_output_free() when done.
 */
typedef struct {
    Intent intent;
    char   language[64];
    char  *code; /* heap-allocated; see compiled_output_free() */
} CompiledOutput;

/* Free the code string inside a CompiledOutput. */
void compiled_output_free(CompiledOutput *out);

/* Return 1 if the compiler handles intent->kind, 0 otherwise. */
int  compiler_can_compile(const Compiler *c, const Intent *intent);

#endif /* GENQL_COMPILER_H */
