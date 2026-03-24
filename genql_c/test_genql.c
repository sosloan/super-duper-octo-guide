/*
 * genql_c/test_genql.c
 * ~~~~~~~~~~~~~~~~~~~~
 * C test suite for the GenQL framework.
 *
 * Mirrors the Python tests in tests/ as closely as the C API allows.
 * Uses a minimal hand-rolled test harness (no external dependencies).
 *
 * Build: make test_genql
 * Run:   make test   (or: ./test_genql)
 */
#include "genql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------ */
/* Minimal test harness                                                 */
/* ------------------------------------------------------------------ */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond) do { \
    if (cond) { \
        g_pass++; \
    } else { \
        g_fail++; \
        fprintf(stderr, "FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

#define CHECK_STR_CONTAINS(haystack, needle) do { \
    if (strstr((haystack), (needle))) { \
        g_pass++; \
    } else { \
        g_fail++; \
        fprintf(stderr, "FAIL  %s:%d  '%s' not in '%s'\n", \
                __FILE__, __LINE__, (needle), (haystack)); \
    } \
} while (0)

#define CHECK_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) == 0) { \
        g_pass++; \
    } else { \
        g_fail++; \
        fprintf(stderr, "FAIL  %s:%d\n  expected: %s\n  got:      %s\n", \
                __FILE__, __LINE__, (b), (a)); \
    } \
} while (0)

/* ------------------------------------------------------------------ */
/* KV store tests                                                       */
/* ------------------------------------------------------------------ */

static void test_kv(void) {
    printf("-- kv --\n");

    KV kv = {0};
    kv_set(&kv, "table", "users");
    CHECK_STR_EQ(kv_get(&kv, "table"), "users");
    CHECK_STR_EQ(kv_get(&kv, "missing"), "");

    kv_set_int(&kv, "count", 42);
    CHECK(kv_get_int(&kv, "count", 0) == 42);
    CHECK(kv_get_int(&kv, "missing", 7) == 7);

    /* Update existing key */
    kv_set(&kv, "table", "orders");
    CHECK_STR_EQ(kv_get(&kv, "table"), "orders");

    /* CSV split */
    char out[8][KV_VAL_MAX];
    int n = kv_split_csv("id,name,age", out, 8);
    CHECK(n == 3);
    CHECK_STR_EQ(out[0], "id");
    CHECK_STR_EQ(out[1], "name");
    CHECK_STR_EQ(out[2], "age");

    /* Empty CSV */
    CHECK(kv_split_csv("", out, 8) == 0);

    /* Prefixed entries */
    KV kv2 = {0};
    kv_set(&kv2, "where.id",   "5");
    kv_set(&kv2, "where.name", "alice");
    kv_set(&kv2, "table",      "users");
    KVPair pairs[8];
    int np = kv_get_prefixed(&kv2, "where.", pairs, 8);
    CHECK(np == 2);
}

/* ------------------------------------------------------------------ */
/* Intent tests                                                         */
/* ------------------------------------------------------------------ */

static void test_intent(void) {
    printf("-- intent --\n");

    /* All five kinds */
    CHECK_STR_EQ(intent_kind_str(GENQL_KIND_DATA),     "data");
    CHECK_STR_EQ(intent_kind_str(GENQL_KIND_COMPUTE),  "compute");
    CHECK_STR_EQ(intent_kind_str(GENQL_KIND_SCHEMA),   "schema");
    CHECK_STR_EQ(intent_kind_str(GENQL_KIND_POLICY),   "policy");
    CHECK_STR_EQ(intent_kind_str(GENQL_KIND_WORKFLOW), "workflow");

    CHECK(intent_kind_from_str("data")     == GENQL_KIND_DATA);
    CHECK(intent_kind_from_str("compute")  == GENQL_KIND_COMPUTE);
    CHECK(intent_kind_from_str("schema")   == GENQL_KIND_SCHEMA);
    CHECK(intent_kind_from_str("policy")   == GENQL_KIND_POLICY);
    CHECK(intent_kind_from_str("workflow") == GENQL_KIND_WORKFLOW);

    /* intent_new */
    KV body = {0};
    kv_set(&body, "table", "t");
    Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
    CHECK(i.kind == GENQL_KIND_DATA);
    CHECK_STR_EQ(i.name, "q");
    CHECK_STR_EQ(kv_get(&i.body, "table"), "t");
    CHECK(i.metadata.n == 0);

    /* repr */
    char buf[128];
    intent_repr(&i, buf, sizeof(buf));
    CHECK_STR_CONTAINS(buf, "data");
    CHECK_STR_CONTAINS(buf, "q");

    /* with_metadata */
    KV meta = {0};
    kv_set(&meta, "author", "alice");
    Intent i2 = intent_with_metadata(&i, &meta);
    CHECK_STR_EQ(kv_get(&i2.metadata, "author"), "alice");
    CHECK(i.metadata.n == 0); /* original untouched */

    /* with_body */
    KV extra = {0};
    kv_set(&extra, "operation", "delete");
    Intent i3 = intent_with_body(&i, &extra);
    CHECK_STR_EQ(kv_get(&i3.body, "operation"), "delete");
    CHECK_STR_EQ(kv_get(&i.body,  "operation"), ""); /* original untouched */
}

/* ------------------------------------------------------------------ */
/* Roles tests                                                          */
/* ------------------------------------------------------------------ */

static void test_roles(void) {
    printf("-- roles --\n");

    CHECK(role_owns(&ROLE_DATA,     GENQL_KIND_DATA));
    CHECK(role_owns(&ROLE_COMPUTE,  GENQL_KIND_COMPUTE));
    CHECK(role_owns(&ROLE_SCHEMA,   GENQL_KIND_SCHEMA));
    CHECK(role_owns(&ROLE_POLICY,   GENQL_KIND_POLICY));
    CHECK(role_owns(&ROLE_WORKFLOW, GENQL_KIND_WORKFLOW));

    /* No cross-ownership */
    CHECK(!role_owns(&ROLE_DATA, GENQL_KIND_COMPUTE));

    CHECK(role_for_kind(GENQL_KIND_DATA)     == &ROLE_DATA);
    CHECK(role_for_kind(GENQL_KIND_COMPUTE)  == &ROLE_COMPUTE);
    CHECK(role_for_kind(GENQL_KIND_SCHEMA)   == &ROLE_SCHEMA);
    CHECK(role_for_kind(GENQL_KIND_POLICY)   == &ROLE_POLICY);
    CHECK(role_for_kind(GENQL_KIND_WORKFLOW) == &ROLE_WORKFLOW);

    CHECK_STR_EQ(ROLE_DATA.target_language,     "sql");
    CHECK_STR_EQ(ROLE_COMPUTE.target_language,  "python");
    CHECK_STR_EQ(ROLE_SCHEMA.target_language,   "json_schema");
    CHECK_STR_EQ(ROLE_POLICY.target_language,   "policy_dsl");
    CHECK_STR_EQ(ROLE_WORKFLOW.target_language, "workflow_dsl");

    char buf[64];
    role_repr(&ROLE_DATA, buf, sizeof(buf));
    CHECK_STR_CONTAINS(buf, "data");
    CHECK_STR_CONTAINS(buf, "sql");
}

/* ------------------------------------------------------------------ */
/* Absorber tests                                                       */
/* ------------------------------------------------------------------ */

static void test_absorber(void) {
    printf("-- absorber --\n");

    /* Empty absorber returns intent unchanged */
    Absorber abs; absorber_init(&abs);
    KV body = {0}; kv_set(&body, "table", "t");
    Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
    Intent result = absorber_absorb(&abs, i);
    CHECK_STR_EQ(result.name, "q");

    /* normalise_name: lowercase + strip */
    Intent i2 = intent_new(GENQL_KIND_DATA, "  GET_USERS  ", &body);
    Intent r2  = mw_normalise_name(i2);
    CHECK_STR_EQ(r2.name, "get_users");

    /* normalise_name: already clean */
    Intent i3 = intent_new(GENQL_KIND_DATA, "get_users", &body);
    Intent r3  = mw_normalise_name(i3);
    CHECK_STR_EQ(r3.name, "get_users");

    /* default_operation: adds "select" when absent */
    KV body4 = {0}; kv_set(&body4, "table", "users");
    Intent i4 = intent_new(GENQL_KIND_DATA, "q", &body4);
    Intent r4  = mw_default_operation(i4);
    CHECK_STR_EQ(kv_get(&r4.body, "operation"), "select");

    /* default_operation: does not override existing */
    KV body5 = {0};
    kv_set(&body5, "table", "users");
    kv_set(&body5, "operation", "delete");
    Intent i5 = intent_new(GENQL_KIND_DATA, "q", &body5);
    Intent r5  = mw_default_operation(i5);
    CHECK_STR_EQ(kv_get(&r5.body, "operation"), "delete");

    /* default_operation: ignores non-DATA */
    KV body6 = {0}; kv_set(&body6, "function", "f");
    Intent i6 = intent_new(GENQL_KIND_COMPUTE, "fn", &body6);
    Intent r6  = mw_default_operation(i6);
    CHECK_STR_EQ(kv_get(&r6.body, "operation"), ""); /* not set */

    /* Chain ordering */
    Absorber abs2; absorber_init(&abs2);
    absorber_use(&abs2, mw_default_operation);
    absorber_use(&abs2, mw_normalise_name);
    KV body7 = {0}; kv_set(&body7, "table", "t");
    Intent i7 = intent_new(GENQL_KIND_DATA, "  Q  ", &body7);
    Intent r7  = absorber_absorb(&abs2, i7);
    CHECK_STR_EQ(r7.name, "q");
    CHECK_STR_EQ(kv_get(&r7.body, "operation"), "select");
}

/* ------------------------------------------------------------------ */
/* SQL compiler tests                                                   */
/* ------------------------------------------------------------------ */

static void test_sql_compiler(void) {
    printf("-- sql_compiler --\n");

    Compiler c = sql_compiler_new();
    CHECK(compiler_can_compile(&c, &(Intent){.kind=GENQL_KIND_DATA}));
    CHECK(!compiler_can_compile(&c, &(Intent){.kind=GENQL_KIND_COMPUTE}));
    CHECK_STR_EQ(c.target_language, "sql");

    /* SELECT * */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        kv_set(&body, "operation", "select");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_EQ(sql, "SELECT * FROM \"users\";");
        free(sql);
    }

    /* SELECT columns */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        kv_set(&body, "operation", "select");
        kv_set(&body, "col", "id,name");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_EQ(sql, "SELECT \"id\", \"name\" FROM \"users\";");
        free(sql);
    }

    /* SELECT with WHERE (integer value) */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        kv_set(&body, "operation", "select");
        kv_set(&body, "where.id", "1");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_EQ(sql, "SELECT * FROM \"users\" WHERE \"id\" = 1;");
        free(sql);
    }

    /* SELECT with WHERE (string value) */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        kv_set(&body, "operation", "select");
        kv_set(&body, "where.name", "alice");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_CONTAINS(sql, "\"name\" = 'alice'");
        free(sql);
    }

    /* INSERT */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        kv_set(&body, "operation", "insert");
        kv_set(&body, "value.name", "bob");
        kv_set(&body, "value.age", "30");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK(strncmp(sql, "INSERT INTO \"users\"", 19) == 0);
        CHECK_STR_CONTAINS(sql, "'bob'");
        CHECK_STR_CONTAINS(sql, "30");
        free(sql);
    }

    /* UPDATE */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        kv_set(&body, "operation", "update");
        kv_set(&body, "value.name", "carol");
        kv_set(&body, "where.id", "5");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_CONTAINS(sql, "UPDATE");
        CHECK_STR_CONTAINS(sql, "'carol'");
        CHECK_STR_CONTAINS(sql, "\"id\" = 5");
        free(sql);
    }

    /* DELETE */
    {
        KV body = {0};
        kv_set(&body, "table", "users");
        kv_set(&body, "operation", "delete");
        kv_set(&body, "where.id", "5");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_EQ(sql, "DELETE FROM \"users\" WHERE \"id\" = 5;");
        free(sql);
    }

    /* Unknown operation returns error string */
    {
        KV body = {0};
        kv_set(&body, "table", "t");
        kv_set(&body, "operation", "drop");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_CONTAINS(sql, "unknown operation");
        free(sql);
    }

    /* SQL injection: single-quote escaping */
    {
        KV body = {0};
        kv_set(&body, "table", "t");
        kv_set(&body, "operation", "select");
        kv_set(&body, "where.name", "O'Brien");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        char *sql = c.compile(&i);
        CHECK_STR_CONTAINS(sql, "O''Brien");
        free(sql);
    }
}

/* ------------------------------------------------------------------ */
/* Python compiler tests                                                */
/* ------------------------------------------------------------------ */

static void test_python_compiler(void) {
    printf("-- python_compiler --\n");

    Compiler c = python_compiler_new();
    CHECK(compiler_can_compile(&c, &(Intent){.kind=GENQL_KIND_COMPUTE}));
    CHECK_STR_EQ(c.target_language, "python");

    /* Expression */
    {
        KV body = {0};
        kv_set(&body, "function",   "double");
        kv_set(&body, "params",     "x");
        kv_set(&body, "expression", "x * 2");
        Intent i = intent_new(GENQL_KIND_COMPUTE, "double", &body);
        char *code = c.compile(&i);
        CHECK_STR_CONTAINS(code, "def double(x):");
        CHECK_STR_CONTAINS(code, "return x * 2");
        free(code);
    }

    /* Steps (multi-statement) */
    {
        KV body = {0};
        kv_set(&body, "function", "greet");
        kv_set(&body, "params",   "name");
        kv_set(&body, "step.0",   "msg = f\"Hello, {name}\"");
        kv_set(&body, "step.1",   "return msg");
        kv_set_int(&body, "step_count", 2);
        Intent i = intent_new(GENQL_KIND_COMPUTE, "greet", &body);
        char *code = c.compile(&i);
        CHECK_STR_CONTAINS(code, "def greet(name):");
        CHECK_STR_CONTAINS(code, "return msg");
        free(code);
    }

    /* Return type annotation */
    {
        KV body = {0};
        kv_set(&body, "function",   "add");
        kv_set(&body, "params",     "a,b");
        kv_set(&body, "expression", "a + b");
        kv_set(&body, "returns",    "int");
        Intent i = intent_new(GENQL_KIND_COMPUTE, "add", &body);
        char *code = c.compile(&i);
        CHECK_STR_CONTAINS(code, "-> int:");
        free(code);
    }

    /* Empty body -> pass */
    {
        KV body = {0};
        Intent i = intent_new(GENQL_KIND_COMPUTE, "noop", &body);
        char *code = c.compile(&i);
        CHECK_STR_CONTAINS(code, "pass");
        free(code);
    }
}

/* ------------------------------------------------------------------ */
/* Schema compiler tests                                                */
/* ------------------------------------------------------------------ */

static void test_schema_compiler(void) {
    printf("-- schema_compiler --\n");

    Compiler c = schema_compiler_new();
    CHECK(compiler_can_compile(&c, &(Intent){.kind=GENQL_KIND_SCHEMA}));
    CHECK_STR_EQ(c.target_language, "json_schema");

    /* Basic schema with properties and required */
    {
        KV body = {0};
        kv_set(&body, "title",          "User");
        kv_set(&body, "prop_names",     "id,name");
        kv_set(&body, "prop.id.type",   "integer");
        kv_set(&body, "prop.name.type", "string");
        kv_set(&body, "required",       "id,name");
        Intent i = intent_new(GENQL_KIND_SCHEMA, "User", &body);
        char *json = c.compile(&i);
        CHECK_STR_CONTAINS(json, "\"title\": \"User\"");
        CHECK_STR_CONTAINS(json, "\"type\": \"object\"");
        CHECK_STR_CONTAINS(json, "\"id\"");
        CHECK_STR_CONTAINS(json, "\"name\"");
        CHECK_STR_CONTAINS(json, "\"additionalProperties\": false");
        CHECK_STR_CONTAINS(json, "2020-12");
        free(json);
    }

    /* No additional properties by default */
    {
        KV body = {0};
        kv_set(&body, "title", "T");
        Intent i = intent_new(GENQL_KIND_SCHEMA, "T", &body);
        char *json = c.compile(&i);
        CHECK_STR_CONTAINS(json, "\"additionalProperties\": false");
        free(json);
    }

    /* additionalProperties true */
    {
        KV body = {0};
        kv_set(&body, "title",     "Open");
        kv_set(&body, "add_props", "true");
        Intent i = intent_new(GENQL_KIND_SCHEMA, "Open", &body);
        char *json = c.compile(&i);
        CHECK_STR_CONTAINS(json, "\"additionalProperties\": true");
        free(json);
    }
}

/* ------------------------------------------------------------------ */
/* Policy compiler tests                                                */
/* ------------------------------------------------------------------ */

static void test_policy_compiler(void) {
    printf("-- policy_compiler --\n");

    Compiler c = policy_compiler_new();
    CHECK(compiler_can_compile(&c, &(Intent){.kind=GENQL_KIND_POLICY}));
    CHECK_STR_EQ(c.target_language, "policy_dsl");

    /* Allow policy */
    {
        KV body = {0};
        kv_set(&body, "subject",  "admin");
        kv_set(&body, "action",   "read");
        kv_set(&body, "resource", "document");
        kv_set(&body, "effect",   "allow");
        Intent i = intent_new(GENQL_KIND_POLICY, "admin_read", &body);
        char *dsl = c.compile(&i);
        CHECK_STR_CONTAINS(dsl, "EFFECT   ALLOW");
        CHECK_STR_CONTAINS(dsl, "SUBJECT  admin");
        CHECK_STR_CONTAINS(dsl, "RESOURCE document");
        free(dsl);
    }

    /* Conditions */
    {
        KV body = {0};
        kv_set(&body, "subject",  "user");
        kv_set(&body, "action",   "write");
        kv_set(&body, "resource", "file");
        kv_set(&body, "effect",   "allow");
        kv_set(&body, "cond.0",   "user.id == file.owner_id");
        kv_set_int(&body, "cond_count", 1);
        Intent i = intent_new(GENQL_KIND_POLICY, "owner_only", &body);
        char *dsl = c.compile(&i);
        CHECK_STR_CONTAINS(dsl, "user.id == file.owner_id");
        free(dsl);
    }

    /* Priority */
    {
        KV body = {0};
        kv_set(&body, "subject",  "s");
        kv_set(&body, "action",   "a");
        kv_set(&body, "resource", "r");
        kv_set(&body, "effect",   "deny");
        kv_set_int(&body, "priority", 10);
        Intent i = intent_new(GENQL_KIND_POLICY, "high_pri", &body);
        char *dsl = c.compile(&i);
        CHECK_STR_CONTAINS(dsl, "priority=10");
        free(dsl);
    }
}

/* ------------------------------------------------------------------ */
/* Workflow compiler tests                                              */
/* ------------------------------------------------------------------ */

static void test_workflow_compiler(void) {
    printf("-- workflow_compiler --\n");

    Compiler c = workflow_compiler_new();
    CHECK(compiler_can_compile(&c, &(Intent){.kind=GENQL_KIND_WORKFLOW}));
    CHECK_STR_EQ(c.target_language, "workflow_dsl");

    /* Basic workflow with depends_on */
    {
        KV body = {0};
        kv_set(&body, "step.0.name",       "validate");
        kv_set(&body, "step.0.role",       "schema");
        kv_set(&body, "step.0.action",     "validate_input");
        kv_set(&body, "step.1.name",       "persist");
        kv_set(&body, "step.1.role",       "data");
        kv_set(&body, "step.1.action",     "insert_user");
        kv_set(&body, "step.1.depends_on", "validate");
        kv_set_int(&body, "step_count", 2);
        Intent i = intent_new(GENQL_KIND_WORKFLOW, "onboard_user", &body);
        char *dsl = c.compile(&i);
        CHECK_STR_CONTAINS(dsl, "WORKFLOW 'onboard_user'");
        CHECK_STR_CONTAINS(dsl, "role:       schema");
        CHECK_STR_CONTAINS(dsl, "depends_on: [validate]");
        free(dsl);
    }

    /* Empty steps */
    {
        KV body = {0};
        kv_set_int(&body, "step_count", 0);
        Intent i = intent_new(GENQL_KIND_WORKFLOW, "empty", &body);
        char *dsl = c.compile(&i);
        CHECK_STR_CONTAINS(dsl, "WORKFLOW 'empty'");
        free(dsl);
    }
}

/* ------------------------------------------------------------------ */
/* Coordinator tests                                                    */
/* ------------------------------------------------------------------ */

static void test_coordinator(void) {
    printf("-- coordinator --\n");

    Coordinator coord = coordinator_default();

    /* All five kinds registered */
    IntentKind kinds[8];
    int nk = coordinator_registered_kinds(&coord, kinds, 8);
    CHECK(nk == 5);

    /* compile DATA -> sql */
    {
        KV body = {0};
        kv_set(&body, "table", "orders");
        Intent i = intent_new(GENQL_KIND_DATA, "get_all", &body);
        CompiledOutput out = coordinator_compile(&coord, &i);
        CHECK_STR_EQ(out.language, "sql");
        CHECK_STR_CONTAINS(out.code, "FROM \"orders\"");
        compiled_output_free(&out);
    }

    /* compile COMPUTE -> python */
    {
        KV body = {0};
        kv_set(&body, "function",   "square");
        kv_set(&body, "params",     "n");
        kv_set(&body, "expression", "n ** 2");
        Intent i = intent_new(GENQL_KIND_COMPUTE, "square", &body);
        CompiledOutput out = coordinator_compile(&coord, &i);
        CHECK_STR_EQ(out.language, "python");
        CHECK_STR_CONTAINS(out.code, "def square(n):");
        compiled_output_free(&out);
    }

    /* compile SCHEMA -> json_schema */
    {
        KV body = {0};
        kv_set(&body, "title", "Product");
        Intent i = intent_new(GENQL_KIND_SCHEMA, "Product", &body);
        CompiledOutput out = coordinator_compile(&coord, &i);
        CHECK_STR_EQ(out.language, "json_schema");
        CHECK_STR_CONTAINS(out.code, "Product");
        compiled_output_free(&out);
    }

    /* compile POLICY -> policy_dsl */
    {
        KV body = {0};
        kv_set(&body, "subject",  "guest");
        kv_set(&body, "action",   "write");
        kv_set(&body, "resource", "any");
        kv_set(&body, "effect",   "deny");
        Intent i = intent_new(GENQL_KIND_POLICY, "deny_guests", &body);
        CompiledOutput out = coordinator_compile(&coord, &i);
        CHECK_STR_EQ(out.language, "policy_dsl");
        CHECK_STR_CONTAINS(out.code, "DENY");
        compiled_output_free(&out);
    }

    /* compile WORKFLOW -> workflow_dsl */
    {
        KV body = {0};
        kv_set(&body, "step.0.name",   "create_user");
        kv_set(&body, "step.0.role",   "data");
        kv_set(&body, "step.0.action", "insert");
        kv_set_int(&body, "step_count", 1);
        Intent i = intent_new(GENQL_KIND_WORKFLOW, "signup", &body);
        CompiledOutput out = coordinator_compile(&coord, &i);
        CHECK_STR_EQ(out.language, "workflow_dsl");
        CHECK_STR_CONTAINS(out.code, "signup");
        compiled_output_free(&out);
    }

    /* compile_all */
    {
        Intent intents[2];
        KV b0 = {0}; kv_set(&b0, "table", "items");
        intents[0] = intent_new(GENQL_KIND_DATA, "list", &b0);

        KV b1 = {0};
        kv_set(&b1, "function",   "fn");
        kv_set(&b1, "expression", "1");
        intents[1] = intent_new(GENQL_KIND_COMPUTE, "fn", &b1);

        CompiledOutput *arr = coordinator_compile_all(&coord, intents, 2);
        CHECK_STR_EQ(arr[0].language, "sql");
        CHECK_STR_EQ(arr[1].language, "python");
        coordinator_compile_all_free(arr, 2);
    }

    /* No compiler registered -> code is NULL */
    {
        Coordinator empty; coordinator_init(&empty);
        KV body = {0}; kv_set(&body, "table", "t");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        CompiledOutput out = coordinator_compile(&empty, &i);
        CHECK(out.code == NULL);
        CHECK(genql_last_error[0] != '\0');
    }

    /* explain() */
    {
        KV body = {0}; kv_set(&body, "table", "users");
        Intent i = intent_new(GENQL_KIND_DATA, "get_users", &body);
        char buf[512];
        coordinator_explain(&coord, &i, buf, sizeof(buf));
        CHECK_STR_CONTAINS(buf, "get_users");
        CHECK_STR_CONTAINS(buf, "data");
        CHECK_STR_CONTAINS(buf, "sql");
    }

    /* Absorber: default_operation middleware adds select */
    {
        KV body = {0}; kv_set(&body, "table", "products");
        Intent i = intent_new(GENQL_KIND_DATA, "q", &body);
        CompiledOutput out = coordinator_compile(&coord, &i);
        CHECK_STR_CONTAINS(out.code, "SELECT");
        compiled_output_free(&out);
    }

    /* Absorber: normalises name */
    {
        KV body = {0}; kv_set(&body, "table", "orders");
        Intent i = intent_new(GENQL_KIND_DATA, "  GET_ORDERS  ", &body);
        CompiledOutput out = coordinator_compile(&coord, &i);
        CHECK_STR_EQ(out.intent.name, "get_orders");
        compiled_output_free(&out);
    }

    /* Languages coexist */
    {
        Intent intents[5];
        KV b[5];
        memset(b, 0, sizeof(b));

        kv_set(&b[0], "table", "t");
        intents[0] = intent_new(GENQL_KIND_DATA, "q", &b[0]);

        kv_set(&b[1], "function", "f"); kv_set(&b[1], "expression", "42");
        intents[1] = intent_new(GENQL_KIND_COMPUTE, "f", &b[1]);

        kv_set(&b[2], "title", "S");
        intents[2] = intent_new(GENQL_KIND_SCHEMA, "S", &b[2]);

        kv_set(&b[3], "subject", "u"); kv_set(&b[3], "action", "r");
        kv_set(&b[3], "resource", "x"); kv_set(&b[3], "effect", "allow");
        intents[3] = intent_new(GENQL_KIND_POLICY, "p", &b[3]);

        kv_set_int(&b[4], "step_count", 0);
        intents[4] = intent_new(GENQL_KIND_WORKFLOW, "w", &b[4]);

        CompiledOutput *arr = coordinator_compile_all(&coord, intents, 5);
        CHECK_STR_EQ(arr[0].language, "sql");
        CHECK_STR_EQ(arr[1].language, "python");
        CHECK_STR_EQ(arr[2].language, "json_schema");
        CHECK_STR_EQ(arr[3].language, "policy_dsl");
        CHECK_STR_EQ(arr[4].language, "workflow_dsl");
        coordinator_compile_all_free(arr, 5);
    }
}

/* ------------------------------------------------------------------ */
/* Entry point                                                          */
/* ------------------------------------------------------------------ */

int main(void) {
    printf("=== GenQL C test suite ===\n");

    test_kv();
    test_intent();
    test_roles();
    test_absorber();
    test_sql_compiler();
    test_python_compiler();
    test_schema_compiler();
    test_policy_compiler();
    test_workflow_compiler();
    test_coordinator();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
