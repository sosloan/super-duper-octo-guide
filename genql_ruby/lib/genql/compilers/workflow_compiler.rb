# genql/compilers/workflow_compiler
#
# Compiles WORKFLOW intents into a step-sequence workflow DSL.
#
# Supported body keys
# -------------------
# steps      : Array<Hash>  — ordered step definitions, each with:
#   name       : String       — step identifier
#   role       : String       — role responsible for this step
#   action     : String       — what the step does
#   depends_on : Array<String> — steps that must complete first (optional)
#   on_failure : String       — "abort" | "continue" (optional)
# on_failure : String       — top-level default failure handling (default "abort")

require_relative "base"
require_relative "../intent"

module Genql
  module Compilers
    class WorkflowCompiler < Compiler
      def target_language
        "workflow_dsl"
      end

      def supported_kinds
        [IntentKind::WORKFLOW]
      end

      def compile(intent)
        body               = intent.body
        steps              = Array(body_get(body, "steps"))
        default_on_failure = body_get(body, "on_failure") || "abort"

        lines = [
          "WORKFLOW '#{intent.name}':",
          "  on_failure: #{default_on_failure}",
          "  steps:"
        ]

        steps.each do |step|
          step_name      = step_get(step, "name")       || "unnamed"
          role           = step_get(step, "role")       || "unknown"
          action         = step_get(step, "action")     || ""
          depends_on     = Array(step_get(step, "depends_on"))
          step_failure   = step_get(step, "on_failure") || default_on_failure

          lines << "    - #{step_name}:"
          lines << "        role:       #{role}"
          lines << "        action:     #{action}"
          lines << "        on_failure: #{step_failure}"
          lines << "        depends_on: [#{depends_on.join(', ')}]" if depends_on.any?
        end

        lines.join("\n")
      end

      private

      def body_get(body, key)
        body[key.to_sym] || body[key.to_s]
      end

      def step_get(step, key)
        step[key.to_sym] || step[key.to_s]
      end
    end
  end
end
