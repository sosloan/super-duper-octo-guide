/*
 * genql_c/kv.h
 * ~~~~~~~~~~~~
 * Simple string key-value store used to represent Intent body and metadata.
 *
 * Convention for complex values
 * ------------------------------
 * Simple string:   kv_set(kv, "table", "users")
 * CSV list:        kv_set(kv, "params", "x,y,z")
 * Prefixed dict:   kv_set(kv, "where.id", "5")
 *                  kv_set(kv, "where.name", "alice")
 * Indexed structs: kv_set(kv, "step.0.name", "validate")
 *                  kv_set_int(kv, "step_count", 2)
 */
#ifndef GENQL_KV_H
#define GENQL_KV_H

#define KV_MAX_ENTRIES 64
#define KV_KEY_MAX     256
#define KV_VAL_MAX     4096

typedef struct {
    char key[KV_KEY_MAX];
    char val[KV_VAL_MAX];
} KVPair;

typedef struct {
    KVPair e[KV_MAX_ENTRIES];
    int    n;
} KV;

/* Set (or update) a key. Silently drops the entry if the store is full. */
void        kv_set(KV *kv, const char *key, const char *val);

/* Set an integer value. */
void        kv_set_int(KV *kv, const char *key, int val);

/* Get a value; returns "" (never NULL) if the key is absent. */
const char *kv_get(const KV *kv, const char *key);

/* Get an integer value; returns def if the key is absent or non-numeric. */
int         kv_get_int(const KV *kv, const char *key, int def);

/*
 * Split a comma-separated value into an array of tokens.
 * out[][KV_VAL_MAX] must have at least max_out rows.
 * Returns the number of tokens found (0 if val is empty).
 */
int kv_split_csv(const char *val, char out[][KV_VAL_MAX], int max_out);

/*
 * Collect all entries whose key starts with prefix.
 * The returned pairs have the prefix stripped from their keys.
 * Returns the count of matching entries.
 */
int kv_get_prefixed(const KV *kv, const char *prefix,
                    KVPair *out, int max_out);

#endif /* GENQL_KV_H */
