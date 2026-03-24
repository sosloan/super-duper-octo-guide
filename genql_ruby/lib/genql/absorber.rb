# genql/absorber
#
# The complexity-absorption layer.
#
# The Absorber is a middleware chain that sits between raw intent and the
# compiler.  Each middleware may enrich, validate, normalise, or annotate
# an Intent before it reaches a compiler.

require_relative "intent"

module Genql
  class Absorber
    def initialize
      @chain = []
    end

    # Register +middleware+ in the chain (returns self for chaining).
    # +middleware+ must respond to +call(intent)+ and return an Intent.
    def use(middleware)
      @chain << middleware
      self
    end

    # Run +intent+ through the full middleware chain.
    def absorb(intent)
      @chain.reduce(intent) { |i, mw| mw.call(i) }
    end

    # ------------------------------------------------------------------ #
    # Built-in middlewares — use as lambdas or pass method references     #
    # ------------------------------------------------------------------ #

    # Lowercase and strip the intent name.
    NORMALISE_NAME = lambda do |intent|
      normalised = intent.name.strip.downcase
      return intent if normalised == intent.name

      Intent.new(intent.kind, normalised, intent.body, intent.metadata)
    end

    # Raise if a DATA intent is missing its +table+ key.
    REQUIRE_TABLE = lambda do |intent|
      if intent.kind == IntentKind::DATA && !intent.body.key?(:table) && !intent.body.key?("table")
        raise ArgumentError,
              "DATA intent #{intent.name.inspect} must include a 'table' in its body."
      end

      intent
    end

    # Set operation to +"select"+ for DATA intents that omit it.
    DEFAULT_OPERATION = lambda do |intent|
      return intent unless intent.kind == IntentKind::DATA
      return intent if intent.body.key?(:operation) || intent.body.key?("operation")

      intent.with_body(operation: "select")
    end

    # Convenience class-method wrappers so callers can write:
    #   absorber.use(Absorber.method(:normalise_name))
    # or just reference the constants directly.

    def self.normalise_name(intent)
      NORMALISE_NAME.call(intent)
    end

    def self.require_table(intent)
      REQUIRE_TABLE.call(intent)
    end

    def self.default_operation(intent)
      DEFAULT_OPERATION.call(intent)
    end
  end
end
