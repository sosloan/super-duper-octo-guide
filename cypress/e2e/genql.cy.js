/**
 * cypress/e2e/genql.cy.js
 * ~~~~~~~~~~~~~~~~~~~~~~~
 * End-to-end test suite for the GenQL Cypress bindings.
 *
 * Each describe block exercises a different IntentKind through the
 * custom cy.genqlCompile / cy.genqlCompileAll / cy.genqlExplain commands.
 */

'use strict';

// Pull the GenQL API from the namespace set up in cypress/support/e2e.js.
const {
  IntentKind,
  createIntent,
  Coordinator,
  NoCompilerRegisteredError,
} = Cypress.genql;

describe('GenQL Cypress bindings', () => {

  // ---------------------------------------------------------------- //
  // DATA → SQL                                                        //
  // ---------------------------------------------------------------- //

  describe('DATA intent compiles to SQL', () => {
    it('compiles a default SELECT intent', () => {
      const intent = createIntent(IntentKind.DATA, 'get_users', { table: 'users' });
      cy.genqlCompile(intent).then((out) => {
        expect(out.language).to.equal('sql');
        expect(out.code).to.include('SELECT');
        expect(out.code).to.include('FROM "users"');
      });
    });

    it('compiles a SELECT with explicit columns and a WHERE clause', () => {
      const intent = createIntent(IntentKind.DATA, 'find_active', {
        table: 'users',
        operation: 'select',
        columns: ['id', 'email'],
        where: { active: 1 },
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.language).to.equal('sql');
        expect(out.code).to.include('"id"');
        expect(out.code).to.include('"email"');
        expect(out.code).to.include('"active" = 1');
      });
    });

    it('compiles an INSERT intent', () => {
      const intent = createIntent(IntentKind.DATA, 'create_user', {
        table: 'users',
        operation: 'insert',
        values: { name: 'Alice', email: 'alice@example.com' },
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.code).to.include('INSERT INTO "users"');
        expect(out.code).to.include("'Alice'");
        expect(out.code).to.include("'alice@example.com'");
      });
    });

    it('compiles an UPDATE intent', () => {
      const intent = createIntent(IntentKind.DATA, 'update_email', {
        table: 'users',
        operation: 'update',
        values: { email: 'new@example.com' },
        where: { id: 1 },
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.code).to.include('UPDATE "users"');
        expect(out.code).to.include("'new@example.com'");
        expect(out.code).to.include('"id" = 1');
      });
    });

    it('compiles a DELETE intent', () => {
      const intent = createIntent(IntentKind.DATA, 'remove_user', {
        table: 'users',
        operation: 'delete',
        where: { id: 99 },
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.code).to.include('DELETE FROM "users"');
        expect(out.code).to.include('"id" = 99');
      });
    });
  });

  // ---------------------------------------------------------------- //
  // COMPUTE → Python                                                  //
  // ---------------------------------------------------------------- //

  describe('COMPUTE intent compiles to Python', () => {
    it('compiles a function definition', () => {
      const intent = createIntent(IntentKind.COMPUTE, 'double', {
        function: 'double',
        params: ['x'],
        expression: 'x * 2',
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.language).to.equal('python');
        expect(out.code).to.include('def double(x):');
        expect(out.code).to.include('return x * 2');
      });
    });

    it('compiles a function with multiple parameters', () => {
      const intent = createIntent(IntentKind.COMPUTE, 'add', {
        function: 'add',
        params: ['a', 'b'],
        expression: 'a + b',
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.code).to.include('def add(a, b):');
        expect(out.code).to.include('return a + b');
      });
    });
  });

  // ---------------------------------------------------------------- //
  // SCHEMA → JSON Schema                                              //
  // ---------------------------------------------------------------- //

  describe('SCHEMA intent compiles to JSON Schema', () => {
    it('compiles a schema with properties', () => {
      const intent = createIntent(IntentKind.SCHEMA, 'Order', {
        title: 'Order',
        properties: {
          id:    { type: 'integer' },
          total: { type: 'number' },
        },
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.language).to.equal('json_schema');
        const schema = JSON.parse(out.code);
        expect(schema.title).to.equal('Order');
        expect(schema.properties).to.have.keys('id', 'total');
        expect(schema.$schema).to.include('json-schema.org');
      });
    });

    it('compiles a schema with required fields', () => {
      const intent = createIntent(IntentKind.SCHEMA, 'User', {
        title: 'User',
        properties: { email: { type: 'string' } },
        required: ['email'],
      });
      cy.genqlCompile(intent).then((out) => {
        const schema = JSON.parse(out.code);
        expect(schema.required).to.deep.equal(['email']);
      });
    });
  });

  // ---------------------------------------------------------------- //
  // POLICY → Policy DSL                                               //
  // ---------------------------------------------------------------- //

  describe('POLICY intent compiles to Policy DSL', () => {
    it('compiles an ALLOW policy', () => {
      const intent = createIntent(IntentKind.POLICY, 'admin_read', {
        subject: 'admin',
        action: 'read',
        resource: 'document',
        effect: 'allow',
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.language).to.equal('policy_dsl');
        expect(out.code).to.include("POLICY 'admin_read'");
        expect(out.code).to.include('ALLOW');
        expect(out.code).to.include('admin');
        expect(out.code).to.include('read');
        expect(out.code).to.include('document');
      });
    });

    it('compiles a DENY policy', () => {
      const intent = createIntent(IntentKind.POLICY, 'guest_deny', {
        subject: 'guest',
        action: 'write',
        resource: 'any',
        effect: 'deny',
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.code).to.include('DENY');
        expect(out.code).to.include('guest');
      });
    });
  });

  // ---------------------------------------------------------------- //
  // WORKFLOW → Workflow DSL                                           //
  // ---------------------------------------------------------------- //

  describe('WORKFLOW intent compiles to Workflow DSL', () => {
    it('compiles a workflow with multiple steps', () => {
      const intent = createIntent(IntentKind.WORKFLOW, 'checkout', {
        steps: [
          { name: 'validate', role: 'schema',  action: 'check' },
          { name: 'charge',   role: 'payment', action: 'run' },
        ],
      });
      cy.genqlCompile(intent).then((out) => {
        expect(out.language).to.equal('workflow_dsl');
        expect(out.code).to.include("WORKFLOW 'checkout'");
        expect(out.code).to.include("STEP 'validate'");
        expect(out.code).to.include("STEP 'charge'");
      });
    });

    it('compiles an empty workflow', () => {
      const intent = createIntent(IntentKind.WORKFLOW, 'noop', { steps: [] });
      cy.genqlCompile(intent).then((out) => {
        expect(out.code).to.include("WORKFLOW 'noop'");
      });
    });
  });

  // ---------------------------------------------------------------- //
  // cy.genqlCompileAll — mixed batch                                  //
  // ---------------------------------------------------------------- //

  describe('cy.genqlCompileAll compiles a mixed batch', () => {
    it('yields outputs in order, one per intent', () => {
      const intents = [
        createIntent(IntentKind.DATA,     'list_orders',  { table: 'orders' }),
        createIntent(IntentKind.COMPUTE,  'total_price',  { function: 'total', expression: 'sum(prices)' }),
        createIntent(IntentKind.SCHEMA,   'OrderSchema',  { title: 'OrderSchema' }),
        createIntent(IntentKind.POLICY,   'read_only',    { subject: 'guest', action: 'write', resource: 'any', effect: 'deny' }),
        createIntent(IntentKind.WORKFLOW, 'signup_flow',  { steps: [] }),
      ];
      cy.genqlCompileAll(intents).then((outputs) => {
        expect(outputs).to.have.length(5);
        const languages = outputs.map(o => o.language);
        expect(languages).to.deep.equal([
          'sql', 'python', 'json_schema', 'policy_dsl', 'workflow_dsl',
        ]);
      });
    });
  });

  // ---------------------------------------------------------------- //
  // Absorber middleware                                               //
  // ---------------------------------------------------------------- //

  describe('Absorber middleware', () => {
    it('normalises intent names to lowercase and trims whitespace', () => {
      const intent = createIntent(IntentKind.DATA, '  GET_ORDERS  ', { table: 'orders' });
      cy.genqlCompile(intent).then((out) => {
        expect(out.intent.name).to.equal('get_orders');
      });
    });

    it('adds a default SELECT operation when none is given', () => {
      const intent = createIntent(IntentKind.DATA, 'list_items', { table: 'items' });
      cy.genqlCompile(intent).then((out) => {
        expect(out.code).to.include('SELECT');
      });
    });
  });

  // ---------------------------------------------------------------- //
  // cy.genqlExplain                                                   //
  // ---------------------------------------------------------------- //

  describe('cy.genqlExplain', () => {
    it('returns a human-readable description of a DATA intent', () => {
      const intent = createIntent(IntentKind.DATA, 'get_users', { table: 'users' });
      cy.genqlExplain(intent).then((text) => {
        expect(text).to.include('get_users');
        expect(text).to.include('data');
        expect(text).to.include('sql');
        expect(text).to.include('SQLCompiler');
      });
    });
  });

  // ---------------------------------------------------------------- //
  // Error handling                                                    //
  // ---------------------------------------------------------------- //

  describe('error handling', () => {
    it('throws NoCompilerRegisteredError for an empty coordinator', () => {
      const emptyCoord = new Coordinator();
      const intent = createIntent(IntentKind.DATA, 'q', { table: 't' });
      expect(() => emptyCoord.compile(intent)).to.throw(NoCompilerRegisteredError);
    });

    it('throws when a DATA intent is missing the table field', () => {
      const { Coordinator: Coord, Absorber: Abs, SQLCompiler: SQL } = Cypress.genql;
      const strictCoord = new Coord(new Abs().use(Abs.requireTable))
        .register(new SQL());
      const intent = createIntent(IntentKind.DATA, 'broken', {});
      expect(() => strictCoord.compile(intent)).to.throw(/must include a 'table'/);
    });
  });

});
