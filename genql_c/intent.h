/*
 * genql_c/intent.h
 * ~~~~~~~~~~~~~~~~
 * Typed declarations of *what* the system wants to accomplish.
 *
 * Every piece of behaviour starts life as an Intent: a named, kind-tagged
 * struct whose body KV carries the domain-specific payload.  The kind
 * determines which compiler will later transform the intent into concrete
 * code.
 */
#ifndef GENQL_INTENT_H
#define GENQL_INTENT_H

#include "kv.h"

/* ------------------------------------------------------------------------ */
/* IntentKind                                                                */
/* ------------------------------------------------------------------------ */

/*
 * The domain categories of intent understood by GenQL.
 *
 * Each kind maps 1-to-1 with a compiler that knows how to express it in the
 * language best suited to that domain.
 */
typedef enum {
    GENQL_KIND_DATA,      /* persistence / queries  -> SQL          */
    GENQL_KIND_COMPUTE,   /* transformation / logic -> C (or Python)*/
    GENQL_KIND_SCHEMA,    /* type contracts         -> JSON Schema  */
    GENQL_KIND_POLICY,    /* rules / constraints    -> policy DSL   */
    GENQL_KIND_WORKFLOW,  /* orchestration steps    -> workflow DSL */
} IntentKind;

/* Return the string representation of a kind (e.g. "data", "compute"). */
const char *intent_kind_str(IntentKind kind);

/* Parse a kind from its string representation; returns GENQL_KIND_DATA on
 * unknown input. */
IntentKind intent_kind_from_str(const char *s);

/* ------------------------------------------------------------------------ */
/* Intent                                                                    */
/* ------------------------------------------------------------------------ */

#define INTENT_NAME_MAX 256

/*
 * An atomic unit of intent.
 *
 * Fields
 * ------
 * kind     : The category of intent (determines which compiler handles it).
 * name     : A human-readable identifier.
 * body     : Domain-specific payload understood by the target compiler.
 * metadata : Optional cross-cutting annotations (author, tags, …).
 *
 * Body / metadata key conventions are documented in kv.h and in the
 * individual compiler headers (sql_compiler.h, etc.).
 */
typedef struct {
    IntentKind kind;
    char       name[INTENT_NAME_MAX];
    KV         body;
    KV         metadata;
} Intent;

/* Construct an Intent.  body may be NULL (empty body). */
Intent intent_new(IntentKind kind, const char *name, const KV *body);

/* Return a copy of i with the additional metadata entries merged in. */
Intent intent_with_metadata(const Intent *i, const KV *extra);

/* Return a copy of i with the additional body entries merged in. */
Intent intent_with_body(const Intent *i, const KV *extra);

/* Return a human-readable one-liner, e.g. "Intent(kind='data', name='q')".
 * buf must be at least buflen bytes. */
const char *intent_repr(const Intent *i, char *buf, int buflen);

#endif /* GENQL_INTENT_H */
