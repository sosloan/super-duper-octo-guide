# genql/compilers/python_compiler
#
# Compiles COMPUTE intents into Python function definitions.
#
# Supported body keys
# -------------------
# function   : String       — name for the generated function
# params     : Array<String> — ordered parameter names
# expression : String       — a single Python expression (pure functions)
# steps      : Array<String> — ordered statement lines (multi-statement bodies)
# returns    : String       — type annotation for the return value (optional)

require_relative "base"
require_relative "../intent"

module Genql
  module Compilers
    class PythonCompiler < Compiler
      def target_language
        "python"
      end

      def supported_kinds
        [IntentKind::COMPUTE]
      end

      def compile(intent)
        body       = intent.body
        fn_name    = body_get(body, "function") || intent.name
        params     = Array(body_get(body, "params"))
        returns    = body_get(body, "returns") || ""
        expression = body_get(body, "expression") || ""
        steps      = Array(body_get(body, "steps"))

        sig      = params.join(", ")
        ret_ann  = returns.empty? ? "" : " -> #{returns}"
        header   = "def #{fn_name}(#{sig})#{ret_ann}:"

        body_lines =
          if steps.any?
            steps
          elsif !expression.empty?
            ["return #{expression}"]
          else
            ["pass"]
          end

        indented = body_lines.map { |line| "    #{line}" }
        ([header] + indented).join("\n")
      end

      private

      def body_get(body, key)
        body[key.to_sym] || body[key.to_s]
      end
    end
  end
end
