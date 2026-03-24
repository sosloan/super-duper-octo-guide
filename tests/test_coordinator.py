"""Tests for genql.coordinator — the coordination engine."""
import pytest
from genql.coordinator import Coordinator, NoCompilerRegisteredError
from genql.intent import Intent, IntentKind
from genql.compilers import (
    SQLCompiler,
    PythonCompiler,
    SchemaCompiler,
    PolicyCompiler,
    WorkflowCompiler,
)


# ------------------------------------------------------------------ #
# Default coordinator fixture                                          #
# ------------------------------------------------------------------ #

@pytest.fixture
def coord():
    return Coordinator.default()


# ------------------------------------------------------------------ #
# Registration                                                         #
# ------------------------------------------------------------------ #

def test_default_registers_all_kinds(coord):
    registered = set(coord.registered_kinds())
    assert registered == set(IntentKind)


def test_register_returns_self():
    c = Coordinator()
    assert c.register(SQLCompiler()) is c


def test_use_returns_self():
    c = Coordinator()
    assert c.use(lambda i: i) is c


# ------------------------------------------------------------------ #
# Compilation — one intent per language                               #
# ------------------------------------------------------------------ #

def test_compile_data(coord):
    i = Intent(IntentKind.DATA, "get_all", {"table": "orders"})
    out = coord.compile(i)
    assert out.language == "sql"
    assert 'FROM "orders"' in out.code


def test_compile_compute(coord):
    i = Intent(
        IntentKind.COMPUTE, "square",
        {"function": "square", "params": ["n"], "expression": "n ** 2"}
    )
    out = coord.compile(i)
    assert out.language == "python"
    assert "def square(n):" in out.code


def test_compile_schema(coord):
    i = Intent(
        IntentKind.SCHEMA, "Product",
        {"title": "Product", "properties": {"sku": {"type": "string"}}}
    )
    out = coord.compile(i)
    assert out.language == "json_schema"
    assert "Product" in out.code


def test_compile_policy(coord):
    i = Intent(
        IntentKind.POLICY, "deny_guests",
        {"subject": "guest", "action": "write", "resource": "any", "effect": "deny"}
    )
    out = coord.compile(i)
    assert out.language == "policy_dsl"
    assert "DENY" in out.code


def test_compile_workflow(coord):
    i = Intent(
        IntentKind.WORKFLOW, "signup",
        {"steps": [{"name": "create_user", "role": "data", "action": "insert"}]}
    )
    out = coord.compile(i)
    assert out.language == "workflow_dsl"
    assert "signup" in out.code


# ------------------------------------------------------------------ #
# compile_all                                                          #
# ------------------------------------------------------------------ #

def test_compile_all(coord):
    intents = [
        Intent(IntentKind.DATA, "list", {"table": "items"}),
        Intent(IntentKind.COMPUTE, "fn", {"function": "fn", "expression": "1"}),
    ]
    outputs = coord.compile_all(intents)
    assert len(outputs) == 2
    assert outputs[0].language == "sql"
    assert outputs[1].language == "python"


# ------------------------------------------------------------------ #
# Error handling                                                       #
# ------------------------------------------------------------------ #

def test_no_compiler_raises():
    c = Coordinator()  # empty registry
    i = Intent(IntentKind.DATA, "q", {"table": "t"})
    with pytest.raises(NoCompilerRegisteredError):
        c.compile(i)


# ------------------------------------------------------------------ #
# explain()                                                            #
# ------------------------------------------------------------------ #

def test_explain(coord):
    i = Intent(IntentKind.DATA, "get_users", {"table": "users"})
    explanation = coord.explain(i)
    assert "get_users" in explanation
    assert "data" in explanation
    assert "sql" in explanation


# ------------------------------------------------------------------ #
# Absorber integration — default_operation middleware                  #
# ------------------------------------------------------------------ #

def test_absorber_adds_default_operation(coord):
    """DATA intent without explicit operation should default to SELECT."""
    i = Intent(IntentKind.DATA, "q", {"table": "products"})
    out = coord.compile(i)
    assert "SELECT" in out.code


def test_absorber_normalises_name(coord):
    i = Intent(IntentKind.DATA, "  GET_ORDERS  ", {"table": "orders"})
    out = coord.compile(i)
    assert out.intent.name == "get_orders"


# ------------------------------------------------------------------ #
# Languages coexist — multiple compilers in same coordinator          #
# ------------------------------------------------------------------ #

def test_languages_coexist(coord):
    """Verify different languages compile correctly side-by-side."""
    outputs = coord.compile_all([
        Intent(IntentKind.DATA, "q", {"table": "t"}),
        Intent(IntentKind.COMPUTE, "f", {"function": "f", "expression": "42"}),
        Intent(IntentKind.SCHEMA, "S", {"title": "S"}),
        Intent(IntentKind.POLICY, "p", {"subject": "u", "action": "r", "resource": "x", "effect": "allow"}),
        Intent(IntentKind.WORKFLOW, "w", {"steps": []}),
    ])
    languages = [o.language for o in outputs]
    assert languages == ["sql", "python", "json_schema", "policy_dsl", "workflow_dsl"]
