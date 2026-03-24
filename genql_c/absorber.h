/*
 * genql_c/absorber.h
 * ~~~~~~~~~~~~~~~~~~
 * The complexity-absorption layer.
 *
 * Inspired by the idea that systems should absorb complexity rather than
 * merely containing it, the Absorber is a middleware chain that sits between
 * raw intent and the compiler.  Each middleware may enrich, validate,
 * normalise, or annotate an Intent before it reaches a compiler.
 *
 * This means individual compilers — and callers — never need to handle
 * cross-cutting concerns such as:
 *   - Name normalisation
 *   - Missing-field defaulting
 *   - Audit / provenance tagging
 *   - Validation gating
 */
#ifndef GENQL_ABSORBER_H
#define GENQL_ABSORBER_H

#include "intent.h"

#define ABSORBER_MAX_MW 16

/* A middleware function transforms one Intent into another (possibly the
 * same, possibly a modified copy). */
typedef Intent (*Middleware)(Intent);

typedef struct {
    Middleware chain[ABSORBER_MAX_MW];
    int        n;
} Absorber;

/* Initialise an empty absorber. */
void   absorber_init(Absorber *abs);

/* Register a middleware in the chain (appended in order). */
void   absorber_use(Absorber *abs, Middleware mw);

/* Run intent through the full middleware chain and return the result. */
Intent absorber_absorb(const Absorber *abs, Intent intent);

/* ------------------------------------------------------------------ */
/* Built-in middlewares                                                 */
/* ------------------------------------------------------------------ */

/* Lowercase and strip the intent name. */
Intent mw_normalise_name(Intent intent);

/*
 * Validate that a DATA intent has a "table" key in its body.
 * If missing, writes an error message to stderr.
 * Callers should check genql_last_error after absorbing.
 */
Intent mw_require_table(Intent intent);

/* Set operation to "select" for DATA intents that omit it. */
Intent mw_default_operation(Intent intent);

/*
 * Global error string set by error-producing middlewares.
 * Check after coordinator_compile() / absorber_absorb() when needed.
 * Reset to "" before each absorb call by the coordinator.
 */
extern char genql_last_error[512];

#endif /* GENQL_ABSORBER_H */
