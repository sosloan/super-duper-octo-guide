/*
 * genql_c/workflow_compiler.c
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles WORKFLOW intents into a step-sequence workflow DSL.
 */
#include "workflow_compiler.h"
#include "strbuf.h"
#include "kv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char *workflow_compile(const Intent *intent) {
    StrBuf sb; sb_init(&sb);
    const KV *body = &intent->body;

    const char *on_failure = kv_get(body, "on_failure");
    int step_count         = kv_get_int(body, "step_count", 0);

    if (on_failure[0] == '\0') on_failure = "abort";

    sb_appendf(&sb, "WORKFLOW '%s':", intent->name);
    sb_appendf(&sb, "\n  on_failure: %s", on_failure);
    sb_append(&sb,  "\n  steps:");

    for (int i = 0; i < step_count; i++) {
        char key[128];

        snprintf(key, sizeof(key), "step.%d.name", i);
        const char *name = kv_get(body, key);
        if (name[0] == '\0') name = "unnamed";

        snprintf(key, sizeof(key), "step.%d.role", i);
        const char *role = kv_get(body, key);
        if (role[0] == '\0') role = "unknown";

        snprintf(key, sizeof(key), "step.%d.action", i);
        const char *action = kv_get(body, key);

        snprintf(key, sizeof(key), "step.%d.on_failure", i);
        const char *step_on_fail = kv_get(body, key);
        if (step_on_fail[0] == '\0') step_on_fail = on_failure;

        snprintf(key, sizeof(key), "step.%d.depends_on", i);
        const char *depends_on = kv_get(body, key);

        sb_appendf(&sb, "\n    - %s:", name);
        sb_appendf(&sb, "\n        role:       %s", role);
        sb_appendf(&sb, "\n        action:     %s", action);
        sb_appendf(&sb, "\n        on_failure: %s", step_on_fail);
        if (depends_on[0] != '\0')
            sb_appendf(&sb, "\n        depends_on: [%s]", depends_on);
    }

    return sb_done(&sb);
}

Compiler workflow_compiler_new(void) {
    Compiler c;
    memset(&c, 0, sizeof(c));
    c.supported_kinds[0] = GENQL_KIND_WORKFLOW;
    c.num_kinds = 1;
    strncpy(c.target_language, "workflow_dsl", sizeof(c.target_language) - 1);
    c.compile = workflow_compile;
    return c;
}
