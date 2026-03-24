"""
genql.compilers.schema_compiler
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Compiles SCHEMA intents into JSON Schema objects (as Python dicts).

Supported body keys
-------------------
title : str
    Human-readable schema title.
description : str  (optional)
    Description of the schema.
properties : dict[str, dict]
    Mapping of property name → property definition.
    Each definition may include ``type``, ``description``,
    ``enum``, ``default``, and any other JSON Schema keywords.
required : list[str]  (optional)
    Names of required properties.
additional_properties : bool  (optional, default False)
    Whether unknown properties are allowed.
"""

from __future__ import annotations

import json
from typing import Any, Dict, List

from genql.compilers.base import Compiler
from genql.intent import Intent, IntentKind


class SchemaCompiler(Compiler):
    """Translates :attr:`IntentKind.SCHEMA` intents to JSON Schema."""

    @property
    def target_language(self) -> str:
        return "json_schema"

    @property
    def supported_kinds(self) -> List[IntentKind]:
        return [IntentKind.SCHEMA]

    def compile(self, intent: Intent) -> str:
        body = intent.body
        schema: Dict[str, Any] = {
            "$schema": "https://json-schema.org/draft/2020-12/schema",
            "title": body.get("title", intent.name),
            "type": "object",
        }
        if "description" in body:
            schema["description"] = body["description"]

        properties = body.get("properties") or {}
        if properties:
            schema["properties"] = properties

        required = body.get("required") or []
        if required:
            schema["required"] = required

        schema["additionalProperties"] = body.get("additional_properties", False)

        return json.dumps(schema, indent=2)
