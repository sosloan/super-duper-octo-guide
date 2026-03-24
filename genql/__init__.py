"""
genql
~~~~~
GenQL — an executable theory of coordination.

The system has multiple kinds of intent, and each kind gets compiled
into the language best suited to express or enforce it.

Quick-start
-----------
>>> from genql import Intent, IntentKind, Coordinator
>>> coord = Coordinator.default()
>>> out = coord.compile(Intent(IntentKind.DATA, "get_users", {"table": "users"}))
>>> print(out.code)
SELECT * FROM "users";
"""

from genql.intent import Intent, IntentKind
from genql.roles import Role, role_for, ALL_ROLES
from genql.absorber import Absorber
from genql.coordinator import Coordinator, NoCompilerRegisteredError
from genql.compilers import (
    Compiler,
    CompiledOutput,
    SQLCompiler,
    PythonCompiler,
    SchemaCompiler,
    PolicyCompiler,
    WorkflowCompiler,
)

__all__ = [
    # core types
    "Intent",
    "IntentKind",
    # roles
    "Role",
    "role_for",
    "ALL_ROLES",
    # complexity absorption
    "Absorber",
    # coordination
    "Coordinator",
    "NoCompilerRegisteredError",
    # compilers
    "Compiler",
    "CompiledOutput",
    "SQLCompiler",
    "PythonCompiler",
    "SchemaCompiler",
    "PolicyCompiler",
    "WorkflowCompiler",
]

__version__ = "0.1.0"
