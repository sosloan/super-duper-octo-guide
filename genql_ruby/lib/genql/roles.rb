# genql/roles
#
# Role definitions — the named *responsibility boundaries* of the system.
#
# A Role is not a compiler; it is the *semantic contract* that says
# "this domain is owned by this language family."  Compilers implement
# roles; the coordinator uses roles to reason about which concerns
# belong together and which must stay separated.

require_relative "intent"

module Genql
  # A named responsibility boundary in the coordination system.
  class Role
    attr_reader :name, :description, :owned_kinds, :target_language

    def initialize(name, description, owned_kinds, target_language)
      @name            = name
      @description     = description
      @owned_kinds     = Array(owned_kinds).freeze
      @target_language = target_language
      freeze
    end

    def owns?(kind)
      @owned_kinds.include?(kind)
    end

    def to_s
      "Role(#{@name.inspect}, language=#{@target_language.inspect})"
    end

    alias inspect to_s
  end

  # Built-in roles
  DATA_ROLE = Role.new(
    "data",
    "Owns persistence, retrieval, and data-shape concerns.",
    [IntentKind::DATA],
    "sql"
  )

  COMPUTE_ROLE = Role.new(
    "compute",
    "Owns transformation, calculation, and side-effect-free logic.",
    [IntentKind::COMPUTE],
    "python"
  )

  SCHEMA_ROLE = Role.new(
    "schema",
    "Owns structural contracts and type enforcement.",
    [IntentKind::SCHEMA],
    "json_schema"
  )

  POLICY_ROLE = Role.new(
    "policy",
    "Owns business rules, access control, and invariants.",
    [IntentKind::POLICY],
    "policy_dsl"
  )

  WORKFLOW_ROLE = Role.new(
    "workflow",
    "Owns orchestration, sequencing, and inter-role coordination.",
    [IntentKind::WORKFLOW],
    "workflow_dsl"
  )

  ALL_ROLES = [DATA_ROLE, COMPUTE_ROLE, SCHEMA_ROLE, POLICY_ROLE, WORKFLOW_ROLE].freeze

  # Return the canonical role responsible for +kind+.
  #
  # @raise [KeyError] if no built-in role owns this kind.
  def self.role_for(kind)
    ALL_ROLES.find { |r| r.owns?(kind) } ||
      raise(KeyError, "No role defined for intent kind #{kind.inspect}")
  end
end
