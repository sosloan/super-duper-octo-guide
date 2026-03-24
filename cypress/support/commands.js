/**
 * cypress/support/commands.js
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Custom Cypress commands for GenQL.
 *
 * Commands
 * --------
 * cy.genqlCompile(intent)
 *   Compile a single GenQL intent using the default coordinator.
 *   Yields { intent, language, code }.
 *
 * cy.genqlCompileAll(intents)
 *   Compile an array of intents using the default coordinator.
 *   Yields an array of { intent, language, code }.
 *
 * cy.genqlExplain(intent)
 *   Return a human-readable description of how an intent will be handled.
 *   Yields a string.
 */

'use strict';

const { Coordinator } = require('./genql');

// A single shared coordinator instance (mirrors Coordinator.default()).
const _coordinator = Coordinator.default();

/**
 * Compile a single GenQL intent.
 * @example
 *   cy.genqlCompile(createIntent(IntentKind.DATA, 'q', { table: 'users' }))
 *     .then(out => expect(out.language).to.equal('sql'));
 */
Cypress.Commands.add('genqlCompile', (intent) => {
  return cy.wrap(_coordinator.compile(intent));
});

/**
 * Compile an array of GenQL intents.
 * @example
 *   cy.genqlCompileAll([intent1, intent2])
 *     .then(outputs => expect(outputs).to.have.length(2));
 */
Cypress.Commands.add('genqlCompileAll', (intents) => {
  return cy.wrap(_coordinator.compileAll(intents));
});

/**
 * Return a human-readable explanation of how an intent would be compiled.
 * @example
 *   cy.genqlExplain(intent).then(text => expect(text).to.include('sql'));
 */
Cypress.Commands.add('genqlExplain', (intent) => {
  return cy.wrap(_coordinator.explain(intent));
});
