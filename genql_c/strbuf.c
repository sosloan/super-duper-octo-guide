/*
 * genql_c/strbuf.c
 * ~~~~~~~~~~~~~~~~
 * Implementation of the dynamically growing string buffer.
 */
#include "strbuf.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define SB_INIT_CAP 512

void sb_init(StrBuf *sb) {
    sb->buf = (char *)malloc(SB_INIT_CAP);
    if (!sb->buf) {
        /* Treat allocation failure as a fatal error. */
        fprintf(stderr, "genql: out of memory in sb_init\n");
        abort();
    }
    sb->buf[0] = '\0';
    sb->len = 0;
    sb->cap = SB_INIT_CAP;
}

void sb_free(StrBuf *sb) {
    free(sb->buf);
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
}

static void sb_ensure(StrBuf *sb, int extra) {
    while (sb->len + extra + 1 > sb->cap) {
        sb->cap *= 2;
        char *tmp = (char *)realloc(sb->buf, sb->cap);
        if (!tmp) {
            fprintf(stderr, "genql: out of memory in sb_ensure\n");
            abort();
        }
        sb->buf = tmp;
    }
}

void sb_append(StrBuf *sb, const char *s) {
    int n = (int)strlen(s);
    sb_ensure(sb, n);
    memcpy(sb->buf + sb->len, s, (size_t)n + 1);
    sb->len += n;
}

void sb_appendf(StrBuf *sb, const char *fmt, ...) {
    char tmp[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    sb_append(sb, tmp);
}

char *sb_done(StrBuf *sb) {
    char *result = sb->buf;
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
    return result;
}
