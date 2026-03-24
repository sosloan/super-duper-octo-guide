"""Tests for all language compilers."""
import json
import pytest
from genql.intent import Intent, IntentKind
from genql.compilers import (
    SQLCompiler,
    PythonCompiler,
    SchemaCompiler,
    PolicyCompiler,
    WorkflowCompiler,
)


# ------------------------------------------------------------------ #
# SQLCompiler                                                          #
# ------------------------------------------------------------------ #

class TestSQLCompiler:
    def setup_method(self):
        self.c = SQLCompiler()

    def test_supported_kind(self):
        assert IntentKind.DATA in self.c.supported_kinds

    def test_target_language(self):
        assert self.c.target_language == "sql"

    def test_select_all(self):
        i = Intent(IntentKind.DATA, "q", {"table": "users", "operation": "select"})
        assert self.c.compile(i) == 'SELECT * FROM "users";'

    def test_select_columns(self):
        i = Intent(
            IntentKind.DATA, "q",
            {"table": "users", "operation": "select", "columns": ["id", "name"]}
        )
        assert self.c.compile(i) == 'SELECT "id", "name" FROM "users";'

    def test_select_where(self):
        i = Intent(
            IntentKind.DATA, "q",
            {"table": "users", "operation": "select", "where": {"id": 1}}
        )
        assert self.c.compile(i) == 'SELECT * FROM "users" WHERE "id" = 1;'

    def test_select_where_string_value(self):
        i = Intent(
            IntentKind.DATA, "q",
            {"table": "users", "operation": "select", "where": {"name": "alice"}}
        )
        assert '"name" = \'alice\'' in self.c.compile(i)

    def test_insert(self):
        i = Intent(
            IntentKind.DATA, "q",
            {"table": "users", "operation": "insert", "values": {"name": "bob", "age": 30}}
        )
        sql = self.c.compile(i)
        assert sql.startswith('INSERT INTO "users"')
        assert "'bob'" in sql
        assert "30" in sql

    def test_update(self):
        i = Intent(
            IntentKind.DATA, "q",
            {
                "table": "users", "operation": "update",
                "values": {"name": "carol"},
                "where": {"id": 5},
            }
        )
        sql = self.c.compile(i)
        assert "UPDATE" in sql
        assert "'carol'" in sql
        assert '"id" = 5' in sql

    def test_delete(self):
        i = Intent(
            IntentKind.DATA, "q",
            {"table": "users", "operation": "delete", "where": {"id": 5}}
        )
        assert self.c.compile(i) == 'DELETE FROM "users" WHERE "id" = 5;'

    def test_unknown_operation_raises(self):
        i = Intent(IntentKind.DATA, "q", {"table": "t", "operation": "drop"})
        with pytest.raises(ValueError, match="unknown operation"):
            self.c.compile(i)

    def test_sql_injection_escaped(self):
        i = Intent(
            IntentKind.DATA, "q",
            {"table": "t", "operation": "select", "where": {"name": "O'Brien"}}
        )
        sql = self.c.compile(i)
        assert "O''Brien" in sql


# ------------------------------------------------------------------ #
# PythonCompiler                                                       #
# ------------------------------------------------------------------ #

class TestPythonCompiler:
    def setup_method(self):
        self.c = PythonCompiler()

    def test_supported_kind(self):
        assert IntentKind.COMPUTE in self.c.supported_kinds

    def test_expression(self):
        i = Intent(
            IntentKind.COMPUTE, "double",
            {"function": "double", "params": ["x"], "expression": "x * 2"}
        )
        code = self.c.compile(i)
        assert "def double(x):" in code
        assert "return x * 2" in code

    def test_steps(self):
        i = Intent(
            IntentKind.COMPUTE, "greet",
            {
                "function": "greet",
                "params": ["name"],
                "steps": ['msg = f"Hello, {name}"', "return msg"],
            }
        )
        code = self.c.compile(i)
        assert "def greet(name):" in code
        assert "return msg" in code

    def test_return_annotation(self):
        i = Intent(
            IntentKind.COMPUTE, "add",
            {"function": "add", "params": ["a", "b"], "expression": "a + b", "returns": "int"}
        )
        code = self.c.compile(i)
        assert "-> int:" in code

    def test_empty_body_generates_pass(self):
        i = Intent(IntentKind.COMPUTE, "noop", {})
        code = self.c.compile(i)
        assert "pass" in code


# ------------------------------------------------------------------ #
# SchemaCompiler                                                       #
# ------------------------------------------------------------------ #

class TestSchemaCompiler:
    def setup_method(self):
        self.c = SchemaCompiler()

    def test_supported_kind(self):
        assert IntentKind.SCHEMA in self.c.supported_kinds

    def test_basic_schema(self):
        i = Intent(
            IntentKind.SCHEMA, "User",
            {
                "title": "User",
                "properties": {
                    "id": {"type": "integer"},
                    "name": {"type": "string"},
                },
                "required": ["id", "name"],
            }
        )
        schema = json.loads(self.c.compile(i))
        assert schema["title"] == "User"
        assert schema["type"] == "object"
        assert schema["required"] == ["id", "name"]
        assert schema["additionalProperties"] is False

    def test_no_additional_properties_default(self):
        i = Intent(IntentKind.SCHEMA, "T", {"title": "T"})
        schema = json.loads(self.c.compile(i))
        assert schema["additionalProperties"] is False

    def test_json_schema_draft(self):
        i = Intent(IntentKind.SCHEMA, "T", {})
        schema = json.loads(self.c.compile(i))
        assert "2020-12" in schema["$schema"]


# ------------------------------------------------------------------ #
# PolicyCompiler                                                       #
# ------------------------------------------------------------------ #

class TestPolicyCompiler:
    def setup_method(self):
        self.c = PolicyCompiler()

    def test_supported_kind(self):
        assert IntentKind.POLICY in self.c.supported_kinds

    def test_allow_policy(self):
        i = Intent(
            IntentKind.POLICY, "admin_read",
            {
                "subject": "admin",
                "action": "read",
                "resource": "document",
                "effect": "allow",
            }
        )
        dsl = self.c.compile(i)
        assert "EFFECT   ALLOW" in dsl
        assert "SUBJECT  admin" in dsl
        assert "RESOURCE document" in dsl

    def test_conditions_included(self):
        i = Intent(
            IntentKind.POLICY, "owner_only",
            {
                "subject": "user",
                "action": "write",
                "resource": "file",
                "effect": "allow",
                "conditions": ["user.id == file.owner_id"],
            }
        )
        dsl = self.c.compile(i)
        assert "user.id == file.owner_id" in dsl

    def test_priority(self):
        i = Intent(
            IntentKind.POLICY, "high_pri",
            {"subject": "s", "action": "a", "resource": "r", "effect": "deny", "priority": 10}
        )
        dsl = self.c.compile(i)
        assert "priority=10" in dsl


# ------------------------------------------------------------------ #
# WorkflowCompiler                                                     #
# ------------------------------------------------------------------ #

class TestWorkflowCompiler:
    def setup_method(self):
        self.c = WorkflowCompiler()

    def test_supported_kind(self):
        assert IntentKind.WORKFLOW in self.c.supported_kinds

    def test_basic_workflow(self):
        i = Intent(
            IntentKind.WORKFLOW, "onboard_user",
            {
                "steps": [
                    {"name": "validate", "role": "schema", "action": "validate_input"},
                    {"name": "persist", "role": "data", "action": "insert_user",
                     "depends_on": ["validate"]},
                ]
            }
        )
        dsl = self.c.compile(i)
        assert "WORKFLOW 'onboard_user'" in dsl
        assert "role:       schema" in dsl
        assert "depends_on: [validate]" in dsl

    def test_empty_steps(self):
        i = Intent(IntentKind.WORKFLOW, "empty", {"steps": []})
        dsl = self.c.compile(i)
        assert "WORKFLOW 'empty'" in dsl
