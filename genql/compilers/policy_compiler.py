"""
genql.compilers.policy_compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Compiles POLICY intents into a declarative policy DSL.

Supported body keys
-------------------
subject : str
    The actor or entity the rule applies to (e.g. ``"user"``, ``"service"``).
action : str
    The operation being governed (e.g. ``"read"``, ``"write"``, ``"delete"``).
resource : str
    The resource being protected (e.g. ``"document"``, ``"account"``).
effect : str
    ``"allow"`` or ``"deny"``.
conditions : list[str]  (optional)
    Human-readable predicate strings that must hold for the effect to apply.
priority : int  (optional, default 0)
    Higher priority rules override lower ones on conflict.
"""

from __future__ import annotations

from typing import List

from genql.compilers.base import Compiler
from genql.intent import Intent, IntentKind


class PolicyCompiler(Compiler):
    """Translates :attr:`IntentKind.POLICY` intents to a policy DSL."""

    @property
    def target_language(self) -> str:
        return "policy_dsl"

    @property
    def supported_kinds(self) -> List[IntentKind]:
        return [IntentKind.POLICY]

    def compile(self, intent: Intent) -> str:
        body = intent.body
        subject = body.get("subject", "any")
        action = body.get("action", "any")
        resource = body.get("resource", "any")
        effect = str(body.get("effect", "deny")).upper()
        conditions: List[str] = list(body.get("conditions") or [])
        priority = int(body.get("priority", 0))

        lines = [
            f"POLICY {intent.name!r} [priority={priority}]:",
            f"  SUBJECT  {subject}",
            f"  ACTION   {action}",
            f"  RESOURCE {resource}",
            f"  EFFECT   {effect}",
        ]
        if conditions:
            lines.append("  WHEN")
            for cond in conditions:
                lines.append(f"    AND {cond}")

        return "\n".join(lines)
