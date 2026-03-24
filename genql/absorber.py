"""
genql.absorber
~~~~~~~~~~~~~~
The complexity-absorption layer.

Inspired by the idea that *systems should absorb complexity rather than
merely containing it*, the :class:`Absorber` is a middleware chain that
sits between raw intent and the compiler.  Each middleware may enrich,
validate, normalise, or annotate an :class:`~genql.intent.Intent` before
it reaches a compiler.

This means individual compilers — and callers — never need to handle
cross-cutting concerns such as:

* Name normalisation
* Missing-field defaulting
* Audit / provenance tagging
* Validation gating

Those concerns are absorbed here, at the seam between expression and
execution.
"""

from __future__ import annotations

from typing import Callable, List

from genql.intent import Intent

Middleware = Callable[[Intent], Intent]


class Absorber:
    """A composable middleware chain for pre-processing intents.

    Middlewares are applied in the order they were registered via
    :meth:`use`.  Each middleware receives an :class:`Intent` and must
    return an :class:`Intent` (possibly a transformed copy).

    Example
    -------
    >>> absorber = Absorber()
    >>> absorber.use(normalise_names)
    >>> absorber.use(add_timestamp)
    >>> processed = absorber.absorb(intent)
    """

    def __init__(self) -> None:
        self._chain: List[Middleware] = []

    # ------------------------------------------------------------------ #
    # Registration                                                         #
    # ------------------------------------------------------------------ #

    def use(self, middleware: Middleware) -> "Absorber":
        """Register *middleware* in the chain (returns self for chaining)."""
        self._chain.append(middleware)
        return self

    # ------------------------------------------------------------------ #
    # Execution                                                            #
    # ------------------------------------------------------------------ #

    def absorb(self, intent: Intent) -> Intent:
        """Run *intent* through the full middleware chain."""
        for middleware in self._chain:
            intent = middleware(intent)
        return intent

    # ------------------------------------------------------------------ #
    # Built-in middlewares (register with .use())                          #
    # ------------------------------------------------------------------ #

    @staticmethod
    def normalise_name(intent: Intent) -> Intent:
        """Lowercase and strip the intent name."""
        normalised = intent.name.strip().lower()
        if normalised == intent.name:
            return intent
        return Intent(
            kind=intent.kind,
            name=normalised,
            body=intent.body,
            metadata=intent.metadata,
        )

    @staticmethod
    def require_table(intent: Intent) -> Intent:
        """Raise if a DATA intent is missing its ``table`` key."""
        from genql.intent import IntentKind
        if intent.kind is IntentKind.DATA and "table" not in intent.body:
            raise ValueError(
                f"DATA intent {intent.name!r} must include a 'table' in its body."
            )
        return intent

    @staticmethod
    def default_operation(intent: Intent) -> Intent:
        """Set operation to ``'select'`` for DATA intents that omit it."""
        from genql.intent import IntentKind
        if intent.kind is IntentKind.DATA and "operation" not in intent.body:
            return intent.with_body(operation="select")
        return intent
