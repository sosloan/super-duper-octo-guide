"""
genql.compilers.python_compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Compiles COMPUTE intents into Python function definitions.

Supported body keys
-------------------
function : str
    Name for the generated function.
params : list[str]
    Ordered parameter names.
expression : str
    A single Python expression that forms the function body.
    Use this for pure transformations (the common case).
steps : list[str]
    Ordered list of statement lines.  Use instead of ``expression``
    when multiple statements are required.
returns : str  (optional)
    Type annotation for the return value.
"""

from __future__ import annotations

from typing import List

from genql.compilers.base import Compiler
from genql.intent import Intent, IntentKind


class PythonCompiler(Compiler):
    """Translates :attr:`IntentKind.COMPUTE` intents to Python source."""

    @property
    def target_language(self) -> str:
        return "python"

    @property
    def supported_kinds(self) -> List[IntentKind]:
        return [IntentKind.COMPUTE]

    def compile(self, intent: Intent) -> str:
        body = intent.body
        fn_name = str(body.get("function", intent.name))
        params: List[str] = list(body.get("params") or [])
        returns: str = body.get("returns", "")
        expression: str = body.get("expression", "")
        steps: List[str] = list(body.get("steps") or [])

        # Build signature
        sig = ", ".join(params)
        ret_ann = f" -> {returns}" if returns else ""
        header = f"def {fn_name}({sig}){ret_ann}:"

        # Build body lines
        if steps:
            body_lines = steps
        elif expression:
            body_lines = [f"return {expression}"]
        else:
            body_lines = ["pass"]

        indented = ["    " + line for line in body_lines]
        return "\n".join([header] + indented)
