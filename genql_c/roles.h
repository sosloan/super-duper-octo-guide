/*
 * genql_c/roles.h
 * ~~~~~~~~~~~~~~~
 * Role definitions — the named responsibility boundaries of the system.
 *
 * A Role is not a compiler; it is the semantic contract that says "this
 * domain is owned by this language family."  Compilers implement roles; the
 * coordinator uses roles to reason about which concerns belong together and
 * which must stay separated.
 */
#ifndef GENQL_ROLES_H
#define GENQL_ROLES_H

#include "intent.h"

#define ROLE_MAX_KINDS 8

/*
 * A named responsibility boundary in the coordination system.
 *
 * Fields
 * ------
 * name            : Short identifier for the role.
 * description     : Human-readable purpose statement.
 * owned_kinds     : The intent kinds this role is authoritative for.
 * num_kinds       : Number of entries in owned_kinds.
 * target_language : The language family best suited to this role.
 */
typedef struct {
    char       name[64];
    char       description[256];
    IntentKind owned_kinds[ROLE_MAX_KINDS];
    int        num_kinds;
    char       target_language[64];
} Role;

/* Return 1 if role owns kind, 0 otherwise. */
int          role_owns(const Role *role, IntentKind kind);

/* Return the canonical Role responsible for kind, or NULL if none. */
const Role  *role_for_kind(IntentKind kind);

/* Format a human-readable string into buf (at least buflen bytes). */
const char  *role_repr(const Role *role, char *buf, int buflen);

/* Built-in roles */
extern const Role ROLE_DATA;
extern const Role ROLE_COMPUTE;
extern const Role ROLE_SCHEMA;
extern const Role ROLE_POLICY;
extern const Role ROLE_WORKFLOW;

#endif /* GENQL_ROLES_H */
