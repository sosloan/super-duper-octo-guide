/**
 * cypress/support/e2e.js
 * ~~~~~~~~~~~~~~~~~~~~~~
 * Cypress E2E support entry point.
 *
 * Imports the GenQL custom commands and exposes the full GenQL API
 * on the Cypress namespace so specs can access it via `Cypress.genql`.
 */

'use strict';

require('./commands');

// Expose the GenQL API for direct use in specs: Cypress.genql.createIntent(...)
Cypress.genql = require('./genql');
