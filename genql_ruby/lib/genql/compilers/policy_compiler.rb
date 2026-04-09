# genql/compilers/policy_compiler
#
# Compiles POLICY intents into a declarative policy DSL.
#
# Supported body keys
# -------------------
# subject    : String       — the actor or entity the rule applies to
# action     : String       — the operation being governed
# resource   : String       — the resource being protected
# effect     : String       — "allow" or "deny"
# conditions : Array<String> — predicates that must hold (optional)
# priority   : Integer      — higher priority overrides lower on conflict (default 0)

require_relative "base"
require_relative "../intent"

module Genql
  module Compilers
    class PolicyCompiler < Compiler
      def target_language
        "policy_dsl"
      end

      def supported_kinds
        [IntentKind::POLICY]
      end

      def compile(intent)
        body       = intent.body
        subject    = body_get(body, "subject")   || "any"
        action     = body_get(body, "action")    || "any"
        resource   = body_get(body, "resource")  || "any"
        effect     = (body_get(body, "effect")   || "deny").to_s.upcase
        conditions = Array(body_get(body, "conditions"))
        priority   = (body_get(body, "priority") || 0).to_i

        lines = [
          "POLICY '#{intent.name}' [priority=#{priority}]:",
          "  SUBJECT  #{subject}",
          "  ACTION   #{action}",
          "  RESOURCE #{resource}",
          "  EFFECT   #{effect}"
        ]

        if conditions.any?
          lines << "  WHEN"
          conditions.each { |cond| lines << "    AND #{cond}" }
        end

        lines.join("\n")
      end

      private

      def body_get(body, key)
        body[key.to_sym] || body[key.to_s]
      end
    end
  end
end
