"""Tests for genql.intent."""
import pytest
from genql.intent import Intent, IntentKind


def test_intent_repr():
    i = Intent(IntentKind.DATA, "get_users", {"table": "users"})
    assert "data" in repr(i)
    assert "get_users" in repr(i)


def test_with_metadata_returns_copy():
    i = Intent(IntentKind.DATA, "q", {"table": "t"})
    i2 = i.with_metadata(author="alice")
    assert i2.metadata["author"] == "alice"
    assert i.metadata == {}  # original untouched


def test_with_body_returns_copy():
    i = Intent(IntentKind.DATA, "q", {"table": "t"})
    i2 = i.with_body(operation="delete")
    assert i2.body["operation"] == "delete"
    assert "operation" not in i.body  # original untouched


def test_all_kinds_exist():
    kinds = {k.value for k in IntentKind}
    assert kinds == {"data", "compute", "schema", "policy", "workflow"}
