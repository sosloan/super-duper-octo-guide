"""Tests for genql.absorber."""
import pytest
from genql.absorber import Absorber
from genql.intent import Intent, IntentKind


def test_absorber_identity():
    """An empty absorber returns the intent unchanged."""
    absorber = Absorber()
    i = Intent(IntentKind.DATA, "q", {"table": "t"})
    assert absorber.absorb(i) is i


def test_absorber_chain_order():
    """Middlewares execute in registration order."""
    log = []
    absorber = Absorber()
    absorber.use(lambda i: (log.append("first"), i)[1])
    absorber.use(lambda i: (log.append("second"), i)[1])
    i = Intent(IntentKind.DATA, "q", {"table": "t"})
    absorber.absorb(i)
    assert log == ["first", "second"]


def test_absorber_use_returns_self():
    absorber = Absorber()
    result = absorber.use(lambda i: i)
    assert result is absorber


def test_normalise_name():
    i = Intent(IntentKind.DATA, "  GET_USERS  ", {"table": "users"})
    result = Absorber.normalise_name(i)
    assert result.name == "get_users"


def test_normalise_name_already_clean():
    i = Intent(IntentKind.DATA, "get_users", {"table": "users"})
    result = Absorber.normalise_name(i)
    assert result.name == "get_users"


def test_default_operation_adds_select():
    i = Intent(IntentKind.DATA, "q", {"table": "users"})
    result = Absorber.default_operation(i)
    assert result.body["operation"] == "select"


def test_default_operation_does_not_override():
    i = Intent(IntentKind.DATA, "q", {"table": "users", "operation": "delete"})
    result = Absorber.default_operation(i)
    assert result.body["operation"] == "delete"


def test_default_operation_ignores_non_data():
    i = Intent(IntentKind.COMPUTE, "fn", {"function": "f"})
    result = Absorber.default_operation(i)
    assert result is i


def test_require_table_passes():
    i = Intent(IntentKind.DATA, "q", {"table": "users"})
    assert Absorber.require_table(i) is i


def test_require_table_raises():
    i = Intent(IntentKind.DATA, "q", {})
    with pytest.raises(ValueError, match="table"):
        Absorber.require_table(i)


def test_require_table_ignores_non_data():
    i = Intent(IntentKind.COMPUTE, "fn", {})
    assert Absorber.require_table(i) is i


def test_full_pipeline():
    """Combined absorber + compiler integration check."""
    from genql.compilers import SQLCompiler

    absorber = (
        Absorber()
        .use(Absorber.normalise_name)
        .use(Absorber.default_operation)
        .use(Absorber.require_table)
    )
    i = Intent(IntentKind.DATA, "  GET_USERS  ", {"table": "users"})
    processed = absorber.absorb(i)
    assert processed.name == "get_users"
    assert processed.body["operation"] == "select"

    sql = SQLCompiler().compile(processed)
    assert 'SELECT * FROM "users";' == sql
