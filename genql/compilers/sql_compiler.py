"""
genql.compilers.sql_compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Compiles DATA intents into SQL statements.

Supported body keys
-------------------
operation : str
    ``"select"`` | ``"insert"`` | ``"update"`` | ``"delete"``
table : str
    Target table name.
columns : list[str]  (select only)
    Columns to retrieve; omit or pass ``["*"]`` for all.
where : dict[str, Any]  (select / update / delete)
    Equality conditions, e.g. ``{"id": 42}``.
values : dict[str, Any]  (insert / update)
    Column → value mapping.
"""

from __future__ import annotations

from typing import Any, Dict, List

from genql.compilers.base import Compiler
from genql.intent import Intent, IntentKind


class SQLCompiler(Compiler):
    """Translates :attr:`IntentKind.DATA` intents to SQL."""

    @property
    def target_language(self) -> str:
        return "sql"

    @property
    def supported_kinds(self) -> List[IntentKind]:
        return [IntentKind.DATA]

    # ------------------------------------------------------------------ #
    # Public API                                                           #
    # ------------------------------------------------------------------ #

    def compile(self, intent: Intent) -> str:
        op = str(intent.body.get("operation", "select")).lower()
        dispatch = {
            "select": self._select,
            "insert": self._insert,
            "update": self._update,
            "delete": self._delete,
        }
        if op not in dispatch:
            raise ValueError(
                f"SQLCompiler: unknown operation {op!r}. "
                f"Expected one of {list(dispatch)!r}."
            )
        return dispatch[op](intent.body)

    # ------------------------------------------------------------------ #
    # Private helpers                                                      #
    # ------------------------------------------------------------------ #

    def _quote(self, identifier: str) -> str:
        return f'"{identifier}"'

    def _literal(self, value: Any) -> str:
        if isinstance(value, str):
            escaped = value.replace("'", "''")
            return f"'{escaped}'"
        if value is None:
            return "NULL"
        return str(value)

    def _where_clause(self, where: Dict[str, Any]) -> str:
        if not where:
            return ""
        parts = [
            f"{self._quote(col)} = {self._literal(val)}"
            for col, val in where.items()
        ]
        return " WHERE " + " AND ".join(parts)

    def _select(self, body: Dict[str, Any]) -> str:
        table = self._quote(body["table"])
        cols = body.get("columns") or ["*"]
        col_list = ", ".join(
            "*" if c == "*" else self._quote(c) for c in cols
        )
        where = self._where_clause(body.get("where") or {})
        return f"SELECT {col_list} FROM {table}{where};"

    def _insert(self, body: Dict[str, Any]) -> str:
        table = self._quote(body["table"])
        values: Dict[str, Any] = body.get("values") or {}
        cols = ", ".join(self._quote(c) for c in values)
        vals = ", ".join(self._literal(v) for v in values.values())
        return f"INSERT INTO {table} ({cols}) VALUES ({vals});"

    def _update(self, body: Dict[str, Any]) -> str:
        table = self._quote(body["table"])
        values: Dict[str, Any] = body.get("values") or {}
        set_clause = ", ".join(
            f"{self._quote(c)} = {self._literal(v)}" for c, v in values.items()
        )
        where = self._where_clause(body.get("where") or {})
        return f"UPDATE {table} SET {set_clause}{where};"

    def _delete(self, body: Dict[str, Any]) -> str:
        table = self._quote(body["table"])
        where = self._where_clause(body.get("where") or {})
        return f"DELETE FROM {table}{where};"
