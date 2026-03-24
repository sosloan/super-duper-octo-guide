/*
 * genql_c/strbuf.h
 * ~~~~~~~~~~~~~~~~
 * Dynamically growing string buffer used by compilers to build output.
 */
#ifndef GENQL_STRBUF_H
#define GENQL_STRBUF_H

#include <stddef.h>

typedef struct {
    char  *buf;
    int    len;
    int    cap;
} StrBuf;

/* Initialise buffer (must call before use). */
void  sb_init(StrBuf *sb);

/* Free internal memory without freeing the StrBuf struct itself. */
void  sb_free(StrBuf *sb);

/* Append a NUL-terminated string. */
void  sb_append(StrBuf *sb, const char *s);

/* Append a formatted string (format string limited to 4096 bytes). */
void  sb_appendf(StrBuf *sb, const char *fmt, ...);

/*
 * Finalise the buffer: returns the heap-allocated string (caller must free)
 * and resets the StrBuf to an empty state.
 */
char *sb_done(StrBuf *sb);

#endif /* GENQL_STRBUF_H */
