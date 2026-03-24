/**
 * cypress/support/genql.js
 * ~~~~~~~~~~~~~~~~~~~~~~~~
 * JavaScript port of the core GenQL concepts for use in Cypress tests.
 *
 * Mirrors the Python genql package:
 *   IntentKind    — enum of domain categories
 *   createIntent  — factory for Intent objects
 *   Absorber      — middleware chain for pre-processing intents
 *   Coordinator   — routing engine
 *   *Compiler     — one compiler per IntentKind
 *
 * This module is intentionally self-contained so that tests run entirely
 * in-process with no dependency on a running Python backend.
 */

'use strict';

// ------------------------------------------------------------------ //
// IntentKind                                                           //
// ------------------------------------------------------------------ //

/** The domain categories of intent understood by GenQL. */
const IntentKind = Object.freeze({
  DATA:     'data',     // persistence / queries   → SQL
  COMPUTE:  'compute',  // transformation / logic  → Python
  SCHEMA:   'schema',   // type contracts          → JSON Schema
  POLICY:   'policy',   // rules / constraints     → Policy DSL
  WORKFLOW: 'workflow', // orchestration steps     → Workflow DSL
});

// ------------------------------------------------------------------ //
// Intent factory                                                       //
// ------------------------------------------------------------------ //

/**
 * Create an Intent object.
 *
 * @param {string} kind     - One of the IntentKind values.
 * @param {string} name     - Human-readable identifier.
 * @param {Object} body     - Domain-specific payload for the compiler.
 * @param {Object} metadata - Optional cross-cutting annotations.
 * @returns {Object} Intent
 */
function createIntent(kind, name, body = {}, metadata = {}) {
  return {
    kind,
    name,
    body: { ...body },
    metadata: { ...metadata },

    /** Return a copy with additional metadata entries. */
    withMetadata(extras) {
      return createIntent(this.kind, this.name, this.body, { ...this.metadata, ...extras });
    },

    /** Return a copy with additional / overridden body keys. */
    withBody(extras) {
      return createIntent(this.kind, this.name, { ...this.body, ...extras }, this.metadata);
    },
  };
}

// ------------------------------------------------------------------ //
// Absorber                                                             //
// ------------------------------------------------------------------ //

/** Composable middleware chain that pre-processes intents before compilation. */
class Absorber {
  constructor() {
    this._chain = [];
  }

  /** Add a middleware function to the chain (returns self). */
  use(middleware) {
    this._chain.push(middleware);
    return this;
  }

  /** Run an intent through every middleware in order. */
  absorb(intent) {
    return this._chain.reduce((i, mw) => mw(i), intent);
  }

  // ---------------------------------------------------------------- //
  // Built-in middlewares (register with .use())                       //
  // ---------------------------------------------------------------- //

  /** Lowercase and strip the intent name. */
  static normaliseName(intent) {
    const normalised = intent.name.trim().toLowerCase();
    if (normalised === intent.name) return intent;
    return createIntent(intent.kind, normalised, intent.body, intent.metadata);
  }

  /** Raise if a DATA intent is missing its 'table' key. */
  static requireTable(intent) {
    if (intent.kind === IntentKind.DATA && !intent.body.table) {
      throw new Error(
        `DATA intent '${intent.name}' must include a 'table' in its body.`,
      );
    }
    return intent;
  }

  /** Set operation to 'select' for DATA intents that omit it. */
  static defaultOperation(intent) {
    if (intent.kind === IntentKind.DATA && !intent.body.operation) {
      return intent.withBody({ operation: 'select' });
    }
    return intent;
  }
}

// ------------------------------------------------------------------ //
// Compilers                                                            //
// ------------------------------------------------------------------ //

/** Translates DATA intents to SQL. */
class SQLCompiler {
  get targetLanguage() { return 'sql'; }
  get supportedKinds() { return [IntentKind.DATA]; }

  compile(intent) {
    const op = (intent.body.operation || 'select').toLowerCase();
    const handlers = {
      select: (b) => this._select(b),
      insert: (b) => this._insert(b),
      update: (b) => this._update(b),
      delete: (b) => this._delete(b),
    };
    if (!handlers[op]) {
      throw new Error(
        `SQLCompiler: unknown operation '${op}'. ` +
        `Expected one of ${Object.keys(handlers).join(', ')}.`,
      );
    }
    return handlers[op](intent.body);
  }

  _quote(identifier) {
    return `"${identifier}"`;
  }

  _literal(value) {
    if (value === null || value === undefined) return 'NULL';
    if (typeof value === 'string') return `'${value.replace(/'/g, "''")}'`;
    return String(value);
  }

  _whereClause(where) {
    if (!where || Object.keys(where).length === 0) return '';
    const parts = Object.entries(where).map(
      ([col, val]) => `${this._quote(col)} = ${this._literal(val)}`,
    );
    return ' WHERE ' + parts.join(' AND ');
  }

  _select(body) {
    const table = this._quote(body.table);
    const cols = (body.columns && body.columns.length > 0 ? body.columns : ['*'])
      .map(c => (c === '*' ? '*' : this._quote(c)));
    const where = this._whereClause(body.where);
    return `SELECT ${cols.join(', ')} FROM ${table}${where};`;
  }

  _insert(body) {
    const table = this._quote(body.table);
    const values = body.values || {};
    const cols = Object.keys(values).map(c => this._quote(c)).join(', ');
    const vals = Object.values(values).map(v => this._literal(v)).join(', ');
    return `INSERT INTO ${table} (${cols}) VALUES (${vals});`;
  }

  _update(body) {
    const table = this._quote(body.table);
    const values = body.values || {};
    const set = Object.entries(values)
      .map(([c, v]) => `${this._quote(c)} = ${this._literal(v)}`)
      .join(', ');
    const where = this._whereClause(body.where);
    return `UPDATE ${table} SET ${set}${where};`;
  }

  _delete(body) {
    const table = this._quote(body.table);
    const where = this._whereClause(body.where);
    return `DELETE FROM ${table}${where};`;
  }
}

/** Translates COMPUTE intents to Python function definitions. */
class PythonCompiler {
  get targetLanguage() { return 'python'; }
  get supportedKinds() { return [IntentKind.COMPUTE]; }

  compile(intent) {
    const fn = intent.body.function || intent.name;
    const params = (intent.body.params || []).join(', ');
    const expr = intent.body.expression || 'None';
    return `def ${fn}(${params}):\n    return ${expr}`;
  }
}

/** Translates SCHEMA intents to JSON Schema documents. */
class SchemaCompiler {
  get targetLanguage() { return 'json_schema'; }
  get supportedKinds() { return [IntentKind.SCHEMA]; }

  compile(intent) {
    const schema = {
      $schema: 'https://json-schema.org/draft/2020-12/schema',
      title: intent.body.title || intent.name,
      type: intent.body.type || 'object',
      properties: intent.body.properties || {},
      required: intent.body.required || [],
    };
    return JSON.stringify(schema, null, 2);
  }
}

/** Translates POLICY intents to a Policy DSL. */
class PolicyCompiler {
  get targetLanguage() { return 'policy_dsl'; }
  get supportedKinds() { return [IntentKind.POLICY]; }

  compile(intent) {
    const { subject, action, resource, effect } = intent.body;
    const priority = intent.body.priority !== undefined ? intent.body.priority : 0;
    const eff = (effect || 'allow').toUpperCase();
    return (
      `POLICY '${intent.name}' [priority=${priority}]: ` +
      `${eff} ${subject} TO ${action} ON ${resource};`
    );
  }
}

/** Translates WORKFLOW intents to a Workflow DSL. */
class WorkflowCompiler {
  get targetLanguage() { return 'workflow_dsl'; }
  get supportedKinds() { return [IntentKind.WORKFLOW]; }

  compile(intent) {
    const steps = (intent.body.steps || [])
      .map((s, i) => {
        const name = s.name || `step_${i + 1}`;
        const role = s.role || 'unknown';
        const action = s.action || 'run';
        return `  STEP '${name}' ROLE ${role} ACTION ${action};`;
      })
      .join('\n');
    return `WORKFLOW '${intent.name}' {\n${steps}\n}`;
  }
}

// ------------------------------------------------------------------ //
// Role definitions                                                     //
// ------------------------------------------------------------------ //

const ROLES = {
  [IntentKind.DATA]:     { name: 'data',     targetLanguage: 'sql',          description: 'Persistence and queries' },
  [IntentKind.COMPUTE]:  { name: 'compute',  targetLanguage: 'python',       description: 'Transformation and logic' },
  [IntentKind.SCHEMA]:   { name: 'schema',   targetLanguage: 'json_schema',  description: 'Type contracts' },
  [IntentKind.POLICY]:   { name: 'policy',   targetLanguage: 'policy_dsl',   description: 'Business rules and access control' },
  [IntentKind.WORKFLOW]: { name: 'workflow', targetLanguage: 'workflow_dsl', description: 'Orchestration and sequencing' },
};

/**
 * Return the canonical Role for a given IntentKind.
 * @param {string} kind - One of the IntentKind values.
 */
function roleFor(kind) {
  const role = ROLES[kind];
  if (!role) throw new Error(`No role defined for kind '${kind}'.`);
  return role;
}

// ------------------------------------------------------------------ //
// NoCompilerRegisteredError                                            //
// ------------------------------------------------------------------ //

class NoCompilerRegisteredError extends Error {
  constructor(kind) {
    super(
      `No compiler registered for intent kind '${kind}'. ` +
      `Register one with coordinator.register(compiler).`,
    );
    this.name = 'NoCompilerRegisteredError';
  }
}

// ------------------------------------------------------------------ //
// Coordinator                                                          //
// ------------------------------------------------------------------ //

/**
 * Routes intents to language-specific compilers.
 *
 * Usage:
 *   const coord = Coordinator.default();
 *   const output = coord.compile(createIntent(IntentKind.DATA, 'q', { table: 't' }));
 *   console.log(output.code); // SELECT * FROM "t";
 */
class Coordinator {
  constructor(absorber = null) {
    this._compilers = new Map();
    this._absorber = absorber || new Absorber();
  }

  /** Register a compiler for every kind it supports (returns self). */
  register(compiler) {
    for (const kind of compiler.supportedKinds) {
      this._compilers.set(kind, compiler);
    }
    return this;
  }

  /** Add a middleware to the absorber chain (returns self). */
  use(middleware) {
    this._absorber.use(middleware);
    return this;
  }

  /**
   * Absorb and compile a single intent.
   * @throws {NoCompilerRegisteredError} if no compiler handles the intent's kind.
   */
  compile(intent) {
    const absorbed = this._absorber.absorb(intent);
    const compiler = this._compilers.get(absorbed.kind);
    if (!compiler) {
      throw new NoCompilerRegisteredError(absorbed.kind);
    }
    const code = compiler.compile(absorbed);
    return { intent: absorbed, language: compiler.targetLanguage, code };
  }

  /** Absorb and compile every intent in the array, in order. */
  compileAll(intents) {
    return intents.map(i => this.compile(i));
  }

  /** Return all intent kinds currently handled by this coordinator. */
  registeredKinds() {
    return [...this._compilers.keys()];
  }

  /** Return the canonical Role for a given kind. */
  roleFor(kind) {
    return roleFor(kind);
  }

  /** Return a human-readable description of how an intent will be handled. */
  explain(intent) {
    try {
      const role = roleFor(intent.kind);
      const compiler = this._compilers.get(intent.kind);
      const compilerName = compiler ? compiler.constructor.name : '(unregistered)';
      return (
        `Intent '${intent.name}' [${intent.kind}]\n` +
        `  Role:     ${role.name} — ${role.description}\n` +
        `  Compiler: ${compilerName}\n` +
        `  Language: ${role.targetLanguage}`
      );
    } catch (_) {
      return `Intent '${intent.name}' [${intent.kind}] — no role defined`;
    }
  }

  /**
   * Return a Coordinator pre-loaded with all built-in compilers
   * and the standard absorber middlewares.
   */
  static default() {
    const absorber = new Absorber()
      .use(Absorber.normaliseName)
      .use(Absorber.defaultOperation)
      .use(Absorber.requireTable);

    return new Coordinator(absorber)
      .register(new SQLCompiler())
      .register(new PythonCompiler())
      .register(new SchemaCompiler())
      .register(new PolicyCompiler())
      .register(new WorkflowCompiler());
  }
}

// ------------------------------------------------------------------ //
// Exports                                                              //
// ------------------------------------------------------------------ //

module.exports = {
  IntentKind,
  createIntent,
  Absorber,
  Coordinator,
  NoCompilerRegisteredError,
  SQLCompiler,
  PythonCompiler,
  SchemaCompiler,
  PolicyCompiler,
  WorkflowCompiler,
  roleFor,
  ROLES,
};
