"""
genql.compilers.workflow_compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Compiles WORKFLOW intents into a step-sequence workflow DSL.

Supported body keys
-------------------
steps : list[dict]
    Ordered step definitions.  Each step is a dict with:

    ``name`` : str
        Step identifier.
    ``role`` : str
        The role responsible for this step (e.g. ``"data"``, ``"compute"``).
    ``action`` : str
        What the step does.
    ``depends_on`` : list[str]  (optional)
        Names of steps that must complete before this one starts.
    ``on_failure`` : str  (optional, ``"abort"`` | ``"continue"``)
        Failure handling policy for this step.

on_failure : str  (optional, default ``"abort"``)
    Top-level default failure handling.
"""

from __future__ import annotations

from typing import Any, Dict, List

from genql.compilers.base import Compiler
from genql.intent import Intent, IntentKind


class WorkflowCompiler(Compiler):
    """Translates :attr:`IntentKind.WORKFLOW` intents to a workflow DSL."""

    @property
    def target_language(self) -> str:
        return "workflow_dsl"

    @property
    def supported_kinds(self) -> List[IntentKind]:
        return [IntentKind.WORKFLOW]

    def compile(self, intent: Intent) -> str:
        body = intent.body
        steps: List[Dict[str, Any]] = list(body.get("steps") or [])
        default_on_failure = body.get("on_failure", "abort")

        lines = [
            f"WORKFLOW {intent.name!r}:",
            f"  on_failure: {default_on_failure}",
            "  steps:",
        ]
        for step in steps:
            step_name = step.get("name", "unnamed")
            role = step.get("role", "unknown")
            action = step.get("action", "")
            depends_on: List[str] = list(step.get("depends_on") or [])
            step_on_failure = step.get("on_failure", default_on_failure)

            lines.append(f"    - {step_name}:")
            lines.append(f"        role:       {role}")
            lines.append(f"        action:     {action}")
            lines.append(f"        on_failure: {step_on_failure}")
            if depends_on:
                dep_str = ", ".join(depends_on)
                lines.append(f"        depends_on: [{dep_str}]")

        return "\n".join(lines)
