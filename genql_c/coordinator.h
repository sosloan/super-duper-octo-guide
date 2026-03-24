/*
 * genql_c/coordinator.h
 * ~~~~~~~~~~~~~~~~~~~~~
 * The coordination engine — the heart of GenQL.
 *
 * The Coordinator is the place where languages are made to coexist and roles
 * are separated.  It:
 *   1. Holds a registry of Compiler instances, each responsible for one or
 *      more IntentKind values.
 *   2. Routes each Intent to the compiler that can handle it (one kind →
 *      one compiler; roles stay separated).
 *   3. Passes every intent through an Absorber before compilation so that
 *      cross-cutting complexity never leaks into individual compilers.
 *   4. Provides a single compile / compile_all surface that callers use
 *      regardless of how many languages are involved.
 */
#ifndef GENQL_COORDINATOR_H
#define GENQL_COORDINATOR_H

#include "absorber.h"
#include "compiler.h"
#include "roles.h"

#define COORDINATOR_MAX_COMPILERS 8

typedef struct {
    Compiler compilers[COORDINATOR_MAX_COMPILERS];
    int      num_compilers;
    Absorber absorber;
} Coordinator;

/* Initialise an empty coordinator with no compilers and an empty absorber. */
void coordinator_init(Coordinator *coord);

/* Register a compiler (for all its supported_kinds). Returns coord. */
Coordinator *coordinator_register(Coordinator *coord, Compiler c);

/* Add a middleware to the absorber chain. Returns coord. */
Coordinator *coordinator_use(Coordinator *coord, Middleware mw);

/*
 * Absorb and compile a single intent.
 *
 * On success  : out->code is heap-allocated; call compiled_output_free()
 *               when done with it.
 * On error    : out->code is NULL and genql_last_error is set.
 */
CompiledOutput coordinator_compile(Coordinator *coord, const Intent *intent);

/*
 * Absorb and compile every intent in intents[0..n-1].
 *
 * Returns a heap-allocated array of n CompiledOutput values.
 * The caller must free each element's code field and then free() the array.
 * Convenience macro: coordinator_compile_all_free(arr, n).
 */
CompiledOutput *coordinator_compile_all(Coordinator *coord,
                                        const Intent *intents, int n);

/* Free the code strings and the array returned by coordinator_compile_all. */
void coordinator_compile_all_free(CompiledOutput *arr, int n);

/* Return all IntentKind values that currently have a registered compiler.
 * Writes at most max values into out[]; returns the actual count. */
int coordinator_registered_kinds(const Coordinator *coord,
                                 IntentKind *out, int max);

/* Return the canonical Role for kind, or NULL if none. */
const Role *coordinator_role_for(IntentKind kind);

/* Write a human-readable explanation of how intent will be handled into buf.
 * Returns buf. */
const char *coordinator_explain(const Coordinator *coord, const Intent *intent,
                                char *buf, int buflen);

/*
 * Return a Coordinator pre-loaded with all five built-in compilers and
 * the standard absorber middlewares (normalise_name, default_operation,
 * require_table).
 */
Coordinator coordinator_default(void);

#endif /* GENQL_COORDINATOR_H */
