"""
genql.compilers.base
~~~~~~~~~~~~~~~~~~~~
Abstract base class shared by all language-specific compilers.
"""

from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import List

from genql.intent import Intent, IntentKind


@dataclass
class CompiledOutput:
    """The result of compiling a single Intent.

    Attributes
    ----------
    intent:
        The original intent that was compiled.
    language:
        The target language identifier (e.g. ``"sql"``, ``"python"``).
    code:
        The compiled source code / expression.
    """

    intent: Intent
    language: str
    code: str

    _REPR_PREVIEW_LEN: int = 60

    def __repr__(self) -> str:
        preview = self.code[: self._REPR_PREVIEW_LEN].replace("\n", " ")
        return f"CompiledOutput(language={self.language!r}, code={preview!r}…)"


class Compiler(ABC):
    """Base class for all GenQL compilers.

    A compiler is responsible for transforming one or more :class:`Intent`
    kinds into concrete code in its *target language*.
    """

    @property
    @abstractmethod
    def target_language(self) -> str:
        """Identifier of the language this compiler emits (e.g. ``"sql"``)."""

    @property
    @abstractmethod
    def supported_kinds(self) -> List[IntentKind]:
        """The intent kinds this compiler is able to handle."""

    @abstractmethod
    def compile(self, intent: Intent) -> str:
        """Compile *intent* and return the generated code as a string."""

    def can_compile(self, intent: Intent) -> bool:
        """Return ``True`` if this compiler handles the given intent."""
        return intent.kind in self.supported_kinds
