# genql/intent
#
# Typed declarations of *what* the system wants to accomplish.
#
# Every piece of behaviour starts life as an Intent: a named, kind-tagged
# object whose +body+ carries the domain-specific payload.  The kind
# determines which compiler will later transform the intent into concrete
# code.

module Genql
  # The domain categories of intent understood by GenQL.
  #
  # Each kind maps 1-to-1 with a compiler that knows how to express it
  # in the language best suited to that domain.
  module IntentKind
    DATA     = :data      # persistence / queries  → SQL
    COMPUTE  = :compute   # transformation / logic → Python
    SCHEMA   = :schema    # type contracts         → JSON Schema
    POLICY   = :policy    # rules / constraints    → policy DSL
    WORKFLOW = :workflow  # orchestration steps    → workflow DSL

    ALL = [DATA, COMPUTE, SCHEMA, POLICY, WORKFLOW].freeze
  end

  # An atomic unit of intent.
  #
  # @param kind [Symbol]  The category of intent (determines which compiler handles it).
  # @param name [String]  A human-readable identifier.
  # @param body [Hash]    Domain-specific payload understood by the target compiler.
  # @param metadata [Hash] Optional cross-cutting information (author, tags, …).
  class Intent
    attr_reader :kind, :name, :body, :metadata

    def initialize(kind, name, body = {}, metadata = {})
      @kind     = kind
      @name     = name
      @body     = body.freeze
      @metadata = metadata.freeze
    end

    # Return a copy of this intent with additional metadata entries.
    def with_metadata(**kwargs)
      Intent.new(@kind, @name, @body, @metadata.merge(kwargs))
    end

    # Return a copy of this intent with additional / overridden body keys.
    def with_body(**kwargs)
      Intent.new(@kind, @name, @body.merge(kwargs), @metadata)
    end

    def to_s
      "Intent(kind=#{@kind.inspect}, name=#{@name.inspect})"
    end

    alias inspect to_s
  end
end
