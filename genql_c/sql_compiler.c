/*
 * genql_c/sql_compiler.c
 * ~~~~~~~~~~~~~~~~~~~~~~
 * Compiles DATA intents into SQL statements.
 */
#include "sql_compiler.h"
#include "strbuf.h"
#include "kv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

/* Append a double-quoted SQL identifier. */
static void append_quoted_id(StrBuf *sb, const char *ident) {
    sb_append(sb, "\"");
    sb_append(sb, ident);
    sb_append(sb, "\"");
}

/* Return 1 if the string represents an integer (optional leading '-'). */
static int looks_numeric(const char *val) {
    if (!val || val[0] == '\0') return 0;
    const char *p = val;
    if (*p == '-') p++;
    if (*p == '\0') return 0;
    for (; *p; p++)
        if (*p < '0' || *p > '9') return 0;
    return 1;
}

/* Append a properly quoted / escaped SQL literal value. */
static void append_literal(StrBuf *sb, const char *val) {
    if (strcmp(val, "NULL") == 0) {
        sb_append(sb, "NULL");
        return;
    }
    if (looks_numeric(val)) {
        sb_append(sb, val);
        return;
    }
    /* String literal: single-quote with '' escaping. */
    sb_append(sb, "'");
    for (const char *p = val; *p; p++) {
        if (*p == '\'') sb_append(sb, "'"); /* escape ' as '' */
        char s[2] = {*p, '\0'};
        sb_append(sb, s);
    }
    sb_append(sb, "'");
}

/* Append a WHERE clause from all "where.COL" entries in body. */
static void append_where(StrBuf *sb, const KV *body) {
    KVPair pairs[KV_MAX_ENTRIES];
    int n = kv_get_prefixed(body, "where.", pairs, KV_MAX_ENTRIES);
    if (n == 0) return;
    sb_append(sb, " WHERE ");
    for (int i = 0; i < n; i++) {
        if (i > 0) sb_append(sb, " AND ");
        append_quoted_id(sb, pairs[i].key);
        sb_append(sb, " = ");
        append_literal(sb, pairs[i].val);
    }
}

/* ------------------------------------------------------------------ */
/* Per-operation compilers                                             */
/* ------------------------------------------------------------------ */

static char *sql_select(const KV *body) {
    StrBuf sb; sb_init(&sb);
    const char *table    = kv_get(body, "table");
    const char *cols_str = kv_get(body, "col");

    sb_append(&sb, "SELECT ");
    if (cols_str[0] == '\0' || strcmp(cols_str, "*") == 0) {
        sb_append(&sb, "*");
    } else {
        char cols[KV_MAX_ENTRIES][KV_VAL_MAX];
        int ncols = kv_split_csv(cols_str, cols, KV_MAX_ENTRIES);
        for (int i = 0; i < ncols; i++) {
            if (i > 0) sb_append(&sb, ", ");
            if (strcmp(cols[i], "*") == 0) sb_append(&sb, "*");
            else append_quoted_id(&sb, cols[i]);
        }
    }
    sb_append(&sb, " FROM ");
    append_quoted_id(&sb, table);
    append_where(&sb, body);
    sb_append(&sb, ";");
    return sb_done(&sb);
}

static char *sql_insert(const KV *body) {
    StrBuf sb; sb_init(&sb);
    const char *table = kv_get(body, "table");
    KVPair vals[KV_MAX_ENTRIES];
    int n = kv_get_prefixed(body, "value.", vals, KV_MAX_ENTRIES);

    sb_append(&sb, "INSERT INTO ");
    append_quoted_id(&sb, table);
    sb_append(&sb, " (");
    for (int i = 0; i < n; i++) {
        if (i > 0) sb_append(&sb, ", ");
        append_quoted_id(&sb, vals[i].key);
    }
    sb_append(&sb, ") VALUES (");
    for (int i = 0; i < n; i++) {
        if (i > 0) sb_append(&sb, ", ");
        append_literal(&sb, vals[i].val);
    }
    sb_append(&sb, ");");
    return sb_done(&sb);
}

static char *sql_update(const KV *body) {
    StrBuf sb; sb_init(&sb);
    const char *table = kv_get(body, "table");
    KVPair vals[KV_MAX_ENTRIES];
    int n = kv_get_prefixed(body, "value.", vals, KV_MAX_ENTRIES);

    sb_append(&sb, "UPDATE ");
    append_quoted_id(&sb, table);
    sb_append(&sb, " SET ");
    for (int i = 0; i < n; i++) {
        if (i > 0) sb_append(&sb, ", ");
        append_quoted_id(&sb, vals[i].key);
        sb_append(&sb, " = ");
        append_literal(&sb, vals[i].val);
    }
    append_where(&sb, body);
    sb_append(&sb, ";");
    return sb_done(&sb);
}

static char *sql_delete(const KV *body) {
    StrBuf sb; sb_init(&sb);
    const char *table = kv_get(body, "table");
    sb_append(&sb, "DELETE FROM ");
    append_quoted_id(&sb, table);
    append_where(&sb, body);
    sb_append(&sb, ";");
    return sb_done(&sb);
}

/* ------------------------------------------------------------------ */
/* Main entry point                                                     */
/* ------------------------------------------------------------------ */

static char *sql_compile(const Intent *intent) {
    const char *op = kv_get(&intent->body, "operation");
    if (strcmp(op, "select") == 0) return sql_select(&intent->body);
    if (strcmp(op, "insert") == 0) return sql_insert(&intent->body);
    if (strcmp(op, "update") == 0) return sql_update(&intent->body);
    if (strcmp(op, "delete") == 0) return sql_delete(&intent->body);

    /* Unknown operation — return an error string. */
    StrBuf sb; sb_init(&sb);
    sb_appendf(&sb,
        "ERROR: SQLCompiler: unknown operation '%s'. "
        "Expected one of ['select', 'insert', 'update', 'delete'].", op);
    return sb_done(&sb);
}

Compiler sql_compiler_new(void) {
    Compiler c;
    memset(&c, 0, sizeof(c));
    c.supported_kinds[0] = GENQL_KIND_DATA;
    c.num_kinds = 1;
    strncpy(c.target_language, "sql", sizeof(c.target_language) - 1);
    c.compile = sql_compile;
    return c;
}
