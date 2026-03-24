"""
genql.coordinator
~~~~~~~~~~~~~~~~~
The coordination engine — the heart of GenQL.

The :class:`Coordinator` is the place where *languages are made to
coexist* and *roles are separated*.  It:

1. Holds a registry of :class:`~genql.compilers.base.Compiler` instances,
   each responsible for one or more :class:`~genql.intent.IntentKind`.
2. Routes each :class:`~genql.intent.Intent` to the compiler that can
   handle it (one kind → one compiler; roles stay separated).
3. Passes every intent through an :class:`~genql.absorber.Absorber`
   *before* compilation so that cross-cutting complexity never leaks into
   individual compilers.
4. Provides a single :meth:`compile` / :meth:`compile_all` surface that
   callers use regardless of how many languages are involved.
"""

from __future__ import annotations

from typing import Dict, Iterable, List, Optional

from genql.absorber import Absorber
from genql.compilers.base import Compiler, CompiledOutput
from genql.intent import Intent, IntentKind
from genql.roles import Role, role_for


class NoCompilerRegisteredError(KeyError):
    """Raised when no compiler has been registered for an intent kind."""


class Coordinator:
    """Routes intents to language-specific compilers.

    Usage
    -----
    >>> coord = Coordinator.default()
    >>> output = coord.compile(my_intent)
    >>> print(output.code)
    """

    def __init__(self, absorber: Optional[Absorber] = None) -> None:
        self._compilers: Dict[IntentKind, Compiler] = {}
        self._absorber: Absorber = absorber if absorber is not None else Absorber()

    # ------------------------------------------------------------------ #
    # Registration                                                         #
    # ------------------------------------------------------------------ #

    def register(self, compiler: Compiler) -> "Coordinator":
        """Register *compiler* for every kind it supports (returns self)."""
        for kind in compiler.supported_kinds:
            self._compilers[kind] = compiler
        return self

    def use(self, middleware) -> "Coordinator":
        """Add a middleware to the absorber chain (returns self)."""
        self._absorber.use(middleware)
        return self

    # ------------------------------------------------------------------ #
    # Compilation                                                          #
    # ------------------------------------------------------------------ #

    def compile(self, intent: Intent) -> CompiledOutput:
        """Absorb and compile a single *intent*.

        Raises
        ------
        NoCompilerRegisteredError
            If no compiler has been registered for the intent's kind.
        """
        intent = self._absorber.absorb(intent)
        compiler = self._compilers.get(intent.kind)
        if compiler is None:
            raise NoCompilerRegisteredError(
                f"No compiler registered for intent kind {intent.kind!r}.  "
                f"Register one with coordinator.register(compiler)."
            )
        code = compiler.compile(intent)
        return CompiledOutput(intent=intent, language=compiler.target_language, code=code)

    def compile_all(self, intents: Iterable[Intent]) -> List[CompiledOutput]:
        """Absorb and compile every intent in *intents*, in order."""
        return [self.compile(i) for i in intents]

    # ------------------------------------------------------------------ #
    # Introspection                                                        #
    # ------------------------------------------------------------------ #

    def registered_kinds(self) -> List[IntentKind]:
        """Return all intent kinds currently handled by this coordinator."""
        return list(self._compilers.keys())

    def role_for(self, kind: IntentKind) -> Role:
        """Return the canonical :class:`~genql.roles.Role` for *kind*."""
        return role_for(kind)

    def explain(self, intent: Intent) -> str:
        """Return a human-readable description of how *intent* will be handled."""
        try:
            role = role_for(intent.kind)
            compiler = self._compilers.get(intent.kind)
            compiler_name = type(compiler).__name__ if compiler else "(unregistered)"
            return (
                f"Intent {intent.name!r} [{intent.kind.value}]\n"
                f"  Role:     {role.name} — {role.description}\n"
                f"  Compiler: {compiler_name}\n"
                f"  Language: {role.target_language}"
            )
        except KeyError:
            return f"Intent {intent.name!r} [{intent.kind.value}] — no role defined"

    # ------------------------------------------------------------------ #
    # Factory                                                              #
    # ------------------------------------------------------------------ #

    @classmethod
    def default(cls) -> "Coordinator":
        """Return a :class:`Coordinator` pre-loaded with all built-in compilers
        and the standard absorber middlewares."""
        from genql.compilers import (
            SQLCompiler,
            PythonCompiler,
            SchemaCompiler,
            PolicyCompiler,
            WorkflowCompiler,
        )

        absorber = (
            Absorber()
            .use(Absorber.normalise_name)
            .use(Absorber.default_operation)
            .use(Absorber.require_table)
        )

        return (
            cls(absorber=absorber)
            .register(SQLCompiler())
            .register(PythonCompiler())
            .register(SchemaCompiler())
            .register(PolicyCompiler())
            .register(WorkflowCompiler())
        )
