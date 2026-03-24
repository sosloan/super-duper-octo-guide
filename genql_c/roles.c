/*
 * genql_c/roles.c
 * ~~~~~~~~~~~~~~~
 * Built-in role definitions and lookup helpers.
 */
#include "roles.h"
#include <string.h>
#include <stdio.h>

const Role ROLE_DATA = {
    "data",
    "Owns persistence, retrieval, and data-shape concerns.",
    {GENQL_KIND_DATA}, 1,
    "sql"
};

const Role ROLE_COMPUTE = {
    "compute",
    "Owns transformation, calculation, and side-effect-free logic.",
    {GENQL_KIND_COMPUTE}, 1,
    "python"
};

const Role ROLE_SCHEMA = {
    "schema",
    "Owns structural contracts and type enforcement.",
    {GENQL_KIND_SCHEMA}, 1,
    "json_schema"
};

const Role ROLE_POLICY = {
    "policy",
    "Owns business rules, access control, and invariants.",
    {GENQL_KIND_POLICY}, 1,
    "policy_dsl"
};

const Role ROLE_WORKFLOW = {
    "workflow",
    "Owns orchestration, sequencing, and inter-role coordination.",
    {GENQL_KIND_WORKFLOW}, 1,
    "workflow_dsl"
};

static const Role *ALL_ROLES[] = {
    &ROLE_DATA, &ROLE_COMPUTE, &ROLE_SCHEMA, &ROLE_POLICY, &ROLE_WORKFLOW
};
#define NUM_ROLES ((int)(sizeof(ALL_ROLES) / sizeof(ALL_ROLES[0])))

int role_owns(const Role *role, IntentKind kind) {
    for (int i = 0; i < role->num_kinds; i++)
        if (role->owned_kinds[i] == kind) return 1;
    return 0;
}

const Role *role_for_kind(IntentKind kind) {
    for (int i = 0; i < NUM_ROLES; i++)
        if (role_owns(ALL_ROLES[i], kind)) return ALL_ROLES[i];
    return NULL;
}

const char *role_repr(const Role *role, char *buf, int buflen) {
    snprintf(buf, buflen, "Role('%s', language='%s')",
             role->name, role->target_language);
    return buf;
}
