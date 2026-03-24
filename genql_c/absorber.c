/*
 * genql_c/absorber.c
 * ~~~~~~~~~~~~~~~~~~
 * Absorber and built-in middleware implementations.
 */
#include "absorber.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Global error state (set by error-producing middlewares). */
char genql_last_error[512] = "";

void absorber_init(Absorber *abs) {
    memset(abs, 0, sizeof(*abs));
}

void absorber_use(Absorber *abs, Middleware mw) {
    if (abs->n < ABSORBER_MAX_MW)
        abs->chain[abs->n++] = mw;
}

Intent absorber_absorb(const Absorber *abs, Intent intent) {
    for (int i = 0; i < abs->n; i++)
        intent = abs->chain[i](intent);
    return intent;
}

Intent mw_normalise_name(Intent intent) {
    char buf[INTENT_NAME_MAX];
    int  src = 0, dst = 0;
    int  changed = 0;

    /* Skip leading whitespace. */
    while (intent.name[src] && isspace((unsigned char)intent.name[src])) {
        src++;
        changed = 1;
    }

    /* Lowercase remaining characters. */
    while (intent.name[src]) {
        char c = intent.name[src++];
        if (isupper((unsigned char)c)) {
            c = (char)tolower((unsigned char)c);
            changed = 1;
        }
        buf[dst++] = c;
    }

    /* Trim trailing whitespace. */
    while (dst > 0 && isspace((unsigned char)buf[dst - 1])) {
        dst--;
        changed = 1;
    }
    buf[dst] = '\0';

    if (changed)
        strncpy(intent.name, buf, INTENT_NAME_MAX - 1);
    return intent;
}

Intent mw_require_table(Intent intent) {
    if (intent.kind == GENQL_KIND_DATA) {
        const char *t = kv_get(&intent.body, "table");
        if (t[0] == '\0') {
            snprintf(genql_last_error, sizeof(genql_last_error),
                     "DATA intent '%s' must include a 'table' in its body.",
                     intent.name);
            fprintf(stderr, "ERROR: %s\n", genql_last_error);
        }
    }
    return intent;
}

Intent mw_default_operation(Intent intent) {
    if (intent.kind == GENQL_KIND_DATA) {
        const char *op = kv_get(&intent.body, "operation");
        if (op[0] == '\0')
            kv_set(&intent.body, "operation", "select");
    }
    return intent;
}
