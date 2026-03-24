/*
 * genql_c/intent.c
 * ~~~~~~~~~~~~~~~~
 * IntentKind helpers and Intent construction.
 */
#include "intent.h"
#include <string.h>
#include <stdio.h>

const char *intent_kind_str(IntentKind kind) {
    switch (kind) {
        case GENQL_KIND_DATA:     return "data";
        case GENQL_KIND_COMPUTE:  return "compute";
        case GENQL_KIND_SCHEMA:   return "schema";
        case GENQL_KIND_POLICY:   return "policy";
        case GENQL_KIND_WORKFLOW: return "workflow";
        default:                  return "unknown";
    }
}

IntentKind intent_kind_from_str(const char *s) {
    if (strcmp(s, "data")     == 0) return GENQL_KIND_DATA;
    if (strcmp(s, "compute")  == 0) return GENQL_KIND_COMPUTE;
    if (strcmp(s, "schema")   == 0) return GENQL_KIND_SCHEMA;
    if (strcmp(s, "policy")   == 0) return GENQL_KIND_POLICY;
    if (strcmp(s, "workflow") == 0) return GENQL_KIND_WORKFLOW;
    return GENQL_KIND_DATA; /* default */
}

Intent intent_new(IntentKind kind, const char *name, const KV *body) {
    Intent i;
    memset(&i, 0, sizeof(i));
    i.kind = kind;
    strncpy(i.name, name, INTENT_NAME_MAX - 1);
    i.name[INTENT_NAME_MAX - 1] = '\0';
    if (body) i.body = *body;
    return i;
}

Intent intent_with_metadata(const Intent *base, const KV *extra) {
    Intent i = *base;
    for (int j = 0; j < extra->n; j++)
        kv_set(&i.metadata, extra->e[j].key, extra->e[j].val);
    return i;
}

Intent intent_with_body(const Intent *base, const KV *extra) {
    Intent i = *base;
    for (int j = 0; j < extra->n; j++)
        kv_set(&i.body, extra->e[j].key, extra->e[j].val);
    return i;
}

const char *intent_repr(const Intent *i, char *buf, int buflen) {
    snprintf(buf, buflen, "Intent(kind='%s', name='%s')",
             intent_kind_str(i->kind), i->name);
    return buf;
}
