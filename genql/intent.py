"""
genql.intent
~~~~~~~~~~~~
Typed declarations of *what* the system wants to accomplish.

Every piece of behaviour starts life as an Intent: a named, kind-tagged
object whose ``body`` carries the domain-specific payload.  The kind
determines which compiler will later transform the intent into concrete
code.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum
from typing import Any, Dict


class IntentKind(Enum):
    """The domain categories of intent understood by GenQL.

    Each kind maps 1-to-1 with a compiler that knows how to express it
    in the language best suited to that domain.
    """

    DATA = "data"         # persistence / queries  → SQL
    COMPUTE = "compute"   # transformation / logic → Python
    SCHEMA = "schema"     # type contracts         → JSON Schema
    POLICY = "policy"     # rules / constraints    → policy DSL
    WORKFLOW = "workflow" # orchestration steps    → workflow DSL


@dataclass
class Intent:
    """An atomic unit of intent.

    Parameters
    ----------
    kind:
        The category of intent (determines which compiler handles it).
    name:
        A human-readable identifier.
    body:
        Domain-specific payload understood by the target compiler.
    metadata:
        Optional cross-cutting information (author, tags, provenance …).
    """

    kind: IntentKind
    name: str
    body: Dict[str, Any]
    metadata: Dict[str, Any] = field(default_factory=dict)

    # ------------------------------------------------------------------ #
    # Fluent helpers                                                       #
    # ------------------------------------------------------------------ #

    def with_metadata(self, **kwargs: Any) -> Intent:
        """Return a copy of this intent with additional metadata entries."""
        return Intent(
            kind=self.kind,
            name=self.name,
            body=self.body,
            metadata={**self.metadata, **kwargs},
        )

    def with_body(self, **kwargs: Any) -> Intent:
        """Return a copy of this intent with additional / overridden body keys."""
        return Intent(
            kind=self.kind,
            name=self.name,
            body={**self.body, **kwargs},
            metadata=self.metadata,
        )

    def __repr__(self) -> str:
        return f"Intent(kind={self.kind.value!r}, name={self.name!r})"
