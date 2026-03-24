/*
 * genql_c/kv.c
 * ~~~~~~~~~~~~
 * Key-value store implementation.
 */
#include "kv.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void kv_set(KV *kv, const char *key, const char *val) {
    /* Update existing entry if present. */
    for (int i = 0; i < kv->n; i++) {
        if (strcmp(kv->e[i].key, key) == 0) {
            strncpy(kv->e[i].val, val, KV_VAL_MAX - 1);
            kv->e[i].val[KV_VAL_MAX - 1] = '\0';
            return;
        }
    }
    /* Add new entry. */
    if (kv->n >= KV_MAX_ENTRIES) return; /* store full; silently drop */
    strncpy(kv->e[kv->n].key, key, KV_KEY_MAX - 1);
    kv->e[kv->n].key[KV_KEY_MAX - 1] = '\0';
    strncpy(kv->e[kv->n].val, val, KV_VAL_MAX - 1);
    kv->e[kv->n].val[KV_VAL_MAX - 1] = '\0';
    kv->n++;
}

void kv_set_int(KV *kv, const char *key, int val) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", val);
    kv_set(kv, key, buf);
}

const char *kv_get(const KV *kv, const char *key) {
    for (int i = 0; i < kv->n; i++)
        if (strcmp(kv->e[i].key, key) == 0)
            return kv->e[i].val;
    return "";
}

int kv_get_int(const KV *kv, const char *key, int def) {
    const char *v = kv_get(kv, key);
    if (v[0] == '\0') return def;
    return atoi(v);
}

int kv_split_csv(const char *val, char out[][KV_VAL_MAX], int max_out) {
    if (!val || val[0] == '\0') return 0;
    int count = 0;
    const char *p = val;
    while (*p && count < max_out) {
        const char *q = strchr(p, ',');
        size_t len = q ? (size_t)(q - p) : strlen(p);
        if (len >= KV_VAL_MAX) len = KV_VAL_MAX - 1;
        memcpy(out[count], p, len);
        out[count][len] = '\0';
        count++;
        if (!q) break;
        p = q + 1;
    }
    return count;
}

int kv_get_prefixed(const KV *kv, const char *prefix,
                    KVPair *out, int max_out) {
    int plen = (int)strlen(prefix);
    int count = 0;
    for (int i = 0; i < kv->n && count < max_out; i++) {
        if (strncmp(kv->e[i].key, prefix, (size_t)plen) == 0) {
            strncpy(out[count].key, kv->e[i].key + plen, KV_KEY_MAX - 1);
            out[count].key[KV_KEY_MAX - 1] = '\0';
            strncpy(out[count].val, kv->e[i].val, KV_VAL_MAX - 1);
            out[count].val[KV_VAL_MAX - 1] = '\0';
            count++;
        }
    }
    return count;
}
