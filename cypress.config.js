const { defineConfig } = require('cypress');

module.exports = defineConfig({
  e2e: {
    specPattern: 'cypress/e2e/**/*.cy.{js,jsx,ts,tsx}',
    supportFile: 'cypress/support/e2e.js',
    // No browser needed — all compilation is pure JS, so we run headlessly.
    setupNodeEvents(on, config) {
      // To integrate with the Python backend, register cy.task() handlers here.
      // Example:
      //   on('task', {
      //     genqlCompile({ kind, name, body }) {
      //       // spawn Python process and return compiled output
      //     },
      //   });
    },
  },
  video: false,
  screenshotOnRunFailure: false,
});
