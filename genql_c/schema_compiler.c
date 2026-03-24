/*
 * genql_c/schema_compiler.c
 * ~~~~~~~~~~~~~~~~~~~~~~~~~
 * Compiles SCHEMA intents into JSON Schema strings.
 */
#include "schema_compiler.h"
#include "strbuf.h"
#include "kv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char *schema_compile(const Intent *intent) {
    StrBuf sb; sb_init(&sb);
    const KV *body = &intent->body;

    const char *title      = kv_get(body, "title");
    const char *desc       = kv_get(body, "description");
    const char *add_props  = kv_get(body, "add_props");
    const char *prop_names = kv_get(body, "prop_names");
    const char *req_str    = kv_get(body, "required");

    if (title[0] == '\0') title = intent->name;

    /* Opening brace */
    sb_append(&sb, "{\n");
    sb_append(&sb, "  \"$schema\": \"https://json-schema.org/draft/2020-12/schema\",\n");
    sb_appendf(&sb, "  \"title\": \"%s\",\n", title);
    if (desc[0] != '\0')
        sb_appendf(&sb, "  \"description\": \"%s\",\n", desc);
    sb_append(&sb, "  \"type\": \"object\"");

    /* Properties */
    if (prop_names[0] != '\0') {
        char names[32][KV_VAL_MAX];
        int nnames = kv_split_csv(prop_names, names, 32);
        sb_append(&sb, ",\n  \"properties\": {\n");
        for (int i = 0; i < nnames; i++) {
            if (i > 0) sb_append(&sb, ",\n");
            /* Collect all fields for this property */
            char prefix[KV_KEY_MAX];
            snprintf(prefix, sizeof(prefix), "prop.%.*s.",
                     (int)(sizeof(prefix) - 7), names[i]);
            KVPair fields[16];
            int nf = kv_get_prefixed(body, prefix, fields, 16);

            sb_appendf(&sb, "    \"%s\": {", names[i]);
            for (int j = 0; j < nf; j++) {
                if (j > 0) sb_append(&sb, ", ");
                sb_appendf(&sb, "\"%s\": \"%s\"", fields[j].key, fields[j].val);
            }
            sb_append(&sb, "}");
        }
        sb_append(&sb, "\n  }");
    }

    /* Required */
    if (req_str[0] != '\0') {
        char reqs[32][KV_VAL_MAX];
        int nr = kv_split_csv(req_str, reqs, 32);
        sb_append(&sb, ",\n  \"required\": [\n");
        for (int i = 0; i < nr; i++) {
            if (i > 0) sb_append(&sb, ",\n");
            sb_appendf(&sb, "    \"%s\"", reqs[i]);
        }
        sb_append(&sb, "\n  ]");
    }

    /* additionalProperties */
    int extra = (add_props[0] != '\0' && strcmp(add_props, "true") == 0);
    sb_appendf(&sb, ",\n  \"additionalProperties\": %s",
               extra ? "true" : "false");

    sb_append(&sb, "\n}");
    return sb_done(&sb);
}

Compiler schema_compiler_new(void) {
    Compiler c;
    memset(&c, 0, sizeof(c));
    c.supported_kinds[0] = GENQL_KIND_SCHEMA;
    c.num_kinds = 1;
    strncpy(c.target_language, "json_schema", sizeof(c.target_language) - 1);
    c.compile = schema_compile;
    return c;
}
