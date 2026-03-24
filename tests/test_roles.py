"""Tests for genql.roles."""
import pytest
from genql.roles import (
    role_for,
    ALL_ROLES,
    DATA_ROLE,
    COMPUTE_ROLE,
    SCHEMA_ROLE,
    POLICY_ROLE,
    WORKFLOW_ROLE,
)
from genql.intent import IntentKind


def test_all_roles_cover_all_kinds():
    covered = set()
    for role in ALL_ROLES:
        covered |= role.owned_kinds
    assert covered == set(IntentKind)


def test_role_for_each_kind():
    assert role_for(IntentKind.DATA) is DATA_ROLE
    assert role_for(IntentKind.COMPUTE) is COMPUTE_ROLE
    assert role_for(IntentKind.SCHEMA) is SCHEMA_ROLE
    assert role_for(IntentKind.POLICY) is POLICY_ROLE
    assert role_for(IntentKind.WORKFLOW) is WORKFLOW_ROLE


def test_role_owns():
    assert DATA_ROLE.owns(IntentKind.DATA)
    assert not DATA_ROLE.owns(IntentKind.COMPUTE)


def test_roles_have_distinct_languages():
    languages = [r.target_language for r in ALL_ROLES]
    assert len(languages) == len(set(languages)), "each role must own a unique language"
