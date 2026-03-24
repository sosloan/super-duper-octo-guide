# GenQL — An Executable Theory of Coordination

> *"The system has multiple kinds of intent, and each kind gets compiled
> into the language best suited to express or enforce it."*

GenQL is a coordination framework for systems that must hold more than
one language at once.  Instead of forcing every concern through a single
notation, GenQL lets you *declare intent* in a neutral, typed form and
then routes each piece of intent to the compiler that knows how to
express or enforce it correctly.

---

## The problem it solves

Large systems accumulate complexity at the seams between languages.  A
service that owns data, computation, type contracts, access policies, and
orchestration logic ends up with:

- SQL strings scattered through application code
- Business rules expressed inconsistently as code, config, and comments
- No single place to ask "why does this rule exist?"

GenQL addresses this by separating *what you mean* from *how it is said*,
and by absorbing cross-cutting complexity (normalisation, validation,
defaults) in a dedicated layer rather than spreading it across every
component.

---

## Core concepts

### Intent

An `Intent` is the atomic unit of meaning in GenQL.  It carries:

| Field      | Purpose                                                |
|------------|--------------------------------------------------------|
| `kind`     | *What category of concern is this?* (see below)        |
| `name`     | Human-readable identifier                              |
| `body`     | Domain-specific payload understood by the compiler     |
| `metadata` | Optional cross-cutting annotations (author, tags, …)  |

### Intent kinds and their languages

| Kind       | Domain                          | Compiled to      |
|------------|---------------------------------|------------------|
| `DATA`     | Persistence / queries           | SQL              |
| `COMPUTE`  | Transformation / logic          | Python           |
| `SCHEMA`   | Type contracts                  | JSON Schema      |
| `POLICY`   | Business rules / access control | Policy DSL       |
| `WORKFLOW` | Orchestration / sequencing      | Workflow DSL     |

### Roles

A `Role` is a *named responsibility boundary*.  Roles are not compilers;
they are the semantic contracts that say "this domain is owned by this
language family."  The coordinator uses roles to reason about which
concerns belong together and which must stay separated.

### Absorber

The `Absorber` is a composable middleware chain that sits between raw
intent and the compiler.  It *absorbs complexity* so that individual
compilers — and callers — never have to handle cross-cutting concerns
such as normalisation, missing-field defaulting, or validation.

### Coordinator

The `Coordinator` is where languages are made to coexist.  It:

1. Holds a registry of compilers, one per `IntentKind`.
2. Routes each intent to the right compiler (roles stay separated).
3. Passes every intent through the absorber first.
4. Presents a single `compile` / `compile_all` surface to callers.

---

## Quick start

```python
from genql import Coordinator, Intent, IntentKind

coord = Coordinator.default()

# DATA → SQL
out = coord.compile(Intent(IntentKind.DATA, "get_users", {"table": "users"}))
print(out.language, out.code)
# sql   SELECT * FROM "users";

# COMPUTE → Python
out = coord.compile(Intent(
    IntentKind.COMPUTE, "double",
    {"function": "double", "params": ["x"], "expression": "x * 2"},
))
print(out.language, out.code)
# python  def double(x):\n    return x * 2

# POLICY → Policy DSL
out = coord.compile(Intent(
    IntentKind.POLICY, "admin_read",
    {"subject": "admin", "action": "read", "resource": "document", "effect": "allow"},
))
print(out.language, out.code)
# policy_dsl  POLICY 'admin_read' [priority=0]: …

# Compile a mixed batch — each intent goes to the right language
outputs = coord.compile_all([
    Intent(IntentKind.DATA,     "list_orders",  {"table": "orders"}),
    Intent(IntentKind.COMPUTE,  "total_price",  {"function": "total", "expression": "sum(prices)"}),
    Intent(IntentKind.SCHEMA,   "OrderSchema",  {"title": "Order", "properties": {"id": {"type": "integer"}}}),
    Intent(IntentKind.POLICY,   "read_only",    {"subject": "guest", "action": "write", "resource": "any", "effect": "deny"}),
    Intent(IntentKind.WORKFLOW, "checkout",     {"steps": [{"name": "validate", "role": "schema", "action": "check"}]}),
])
for o in outputs:
    print(f"[{o.language}]  {o.intent.name}")
```

### Custom absorber middleware

```python
from genql import Coordinator, Absorber, Intent, IntentKind

def add_audit_tag(intent: Intent) -> Intent:
    return intent.with_metadata(audited=True)

coord = (
    Coordinator()
    .use(Absorber.normalise_name)
    .use(add_audit_tag)
    # … register compilers …
)
```

### Custom compiler

```python
from genql.compilers.base import Compiler
from genql.intent import Intent, IntentKind

class GraphQLCompiler(Compiler):
    target_language = "graphql"
    supported_kinds = [IntentKind.DATA]

    def compile(self, intent: Intent) -> str:
        table = intent.body["table"]
        return f"query {{ {table} {{ id }} }}"

coord = Coordinator().register(GraphQLCompiler())
```

---

## Running the tests

```bash
pip install pytest
pytest tests/ -v
```

---

## Package layout

```
genql/
  __init__.py          public API surface
  intent.py            Intent + IntentKind
  roles.py             Role definitions
  absorber.py          Complexity-absorption middleware chain
  coordinator.py       Coordination engine
  compilers/
    base.py            Compiler ABC + CompiledOutput
    sql_compiler.py    DATA → SQL
    python_compiler.py COMPUTE → Python
    schema_compiler.py SCHEMA → JSON Schema
    policy_compiler.py POLICY → Policy DSL
    workflow_compiler.py WORKFLOW → Workflow DSL
tests/
  test_intent.py
  test_roles.py
  test_absorber.py
  test_compilers.py
  test_coordinator.py
```