"""
genql.roles
~~~~~~~~~~~
Role definitions — the named *responsibility boundaries* of the system.

A Role is not a compiler; it is the *semantic contract* that says
"this domain is owned by this language family."  Compilers implement
roles; the coordinator uses roles to reason about which concerns
belong together and which must stay separated.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import FrozenSet, List

from genql.intent import IntentKind


@dataclass(frozen=True)
class Role:
    """A named responsibility boundary in the coordination system.

    Attributes
    ----------
    name:
        Short identifier for the role.
    description:
        Human-readable purpose statement.
    owned_kinds:
        The intent kinds this role is authoritative for.
    target_language:
        The language family best suited to express this role's concerns.
    """

    name: str
    description: str
    owned_kinds: FrozenSet[IntentKind]
    target_language: str

    def owns(self, kind: IntentKind) -> bool:
        return kind in self.owned_kinds

    def __repr__(self) -> str:
        return f"Role({self.name!r}, language={self.target_language!r})"


# ------------------------------------------------------------------ #
# Built-in roles                                                       #
# ------------------------------------------------------------------ #

DATA_ROLE = Role(
    name="data",
    description="Owns persistence, retrieval, and data-shape concerns.",
    owned_kinds=frozenset({IntentKind.DATA}),
    target_language="sql",
)

COMPUTE_ROLE = Role(
    name="compute",
    description="Owns transformation, calculation, and side-effect-free logic.",
    owned_kinds=frozenset({IntentKind.COMPUTE}),
    target_language="python",
)

SCHEMA_ROLE = Role(
    name="schema",
    description="Owns structural contracts and type enforcement.",
    owned_kinds=frozenset({IntentKind.SCHEMA}),
    target_language="json_schema",
)

POLICY_ROLE = Role(
    name="policy",
    description="Owns business rules, access control, and invariants.",
    owned_kinds=frozenset({IntentKind.POLICY}),
    target_language="policy_dsl",
)

WORKFLOW_ROLE = Role(
    name="workflow",
    description="Owns orchestration, sequencing, and inter-role coordination.",
    owned_kinds=frozenset({IntentKind.WORKFLOW}),
    target_language="workflow_dsl",
)

ALL_ROLES: List[Role] = [
    DATA_ROLE,
    COMPUTE_ROLE,
    SCHEMA_ROLE,
    POLICY_ROLE,
    WORKFLOW_ROLE,
]


def role_for(kind: IntentKind) -> Role:
    """Return the canonical role responsible for *kind*.

    Raises
    ------
    KeyError
        If no built-in role owns this kind.
    """
    for role in ALL_ROLES:
        if role.owns(kind):
            return role
    raise KeyError(f"No role defined for intent kind {kind!r}")
