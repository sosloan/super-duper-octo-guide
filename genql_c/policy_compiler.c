/*
 * genql_c/policy_compiler.c
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles POLICY intents into a declarative policy DSL.
 */
#include "policy_compiler.h"
#include "strbuf.h"
#include "kv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Convert a string to uppercase in-place (for effect: "allow" -> "ALLOW"). */
static void str_upper(char *dst, const char *src, int max) {
    int i;
    for (i = 0; src[i] && i < max - 1; i++)
        dst[i] = (char)toupper((unsigned char)src[i]);
    dst[i] = '\0';
}

static char *policy_compile(const Intent *intent) {
    StrBuf sb; sb_init(&sb);
    const KV *body = &intent->body;

    const char *subject  = kv_get(body, "subject");
    const char *action   = kv_get(body, "action");
    const char *resource = kv_get(body, "resource");
    const char *effect   = kv_get(body, "effect");
    int priority         = kv_get_int(body, "priority", 0);
    int cond_count       = kv_get_int(body, "cond_count", 0);

    if (subject[0]  == '\0') subject  = "any";
    if (action[0]   == '\0') action   = "any";
    if (resource[0] == '\0') resource = "any";
    if (effect[0]   == '\0') effect   = "deny";

    char effect_upper[64];
    str_upper(effect_upper, effect, (int)sizeof(effect_upper));

    sb_appendf(&sb, "POLICY '%s' [priority=%d]:", intent->name, priority);
    sb_appendf(&sb, "\n  SUBJECT  %s", subject);
    sb_appendf(&sb, "\n  ACTION   %s", action);
    sb_appendf(&sb, "\n  RESOURCE %s", resource);
    sb_appendf(&sb, "\n  EFFECT   %s", effect_upper);

    if (cond_count > 0) {
        sb_append(&sb, "\n  WHEN");
        for (int i = 0; i < cond_count; i++) {
            char key[64];
            snprintf(key, sizeof(key), "cond.%d", i);
            const char *cond = kv_get(body, key);
            sb_appendf(&sb, "\n    AND %s", cond);
        }
    }

    return sb_done(&sb);
}

Compiler policy_compiler_new(void) {
    Compiler c;
    memset(&c, 0, sizeof(c));
    c.supported_kinds[0] = GENQL_KIND_POLICY;
    c.num_kinds = 1;
    strncpy(c.target_language, "policy_dsl", sizeof(c.target_language) - 1);
    c.compile = policy_compile;
    return c;
}
