# genql/compilers/schema_compiler
#
# Compiles SCHEMA intents into JSON Schema objects (as JSON strings).
#
# Supported body keys
# -------------------
# title                 : String  — human-readable schema title
# description           : String  — description (optional)
# properties            : Hash    — property name → property definition
# required              : Array   — names of required properties (optional)
# additional_properties : Boolean — whether unknown properties are allowed (default false)

require "json"
require_relative "base"
require_relative "../intent"

module Genql
  module Compilers
    class SchemaCompiler < Compiler
      def target_language
        "json_schema"
      end

      def supported_kinds
        [IntentKind::SCHEMA]
      end

      def compile(intent)
        body = intent.body

        schema = {
          "$schema"    => "https://json-schema.org/draft/2020-12/schema",
          "title"      => body_get(body, "title") || intent.name,
          "type"       => "object"
        }

        description = body_get(body, "description")
        schema["description"] = description if description

        properties = body_get(body, "properties") || {}
        schema["properties"] = properties unless properties.empty?

        required = body_get(body, "required") || []
        schema["required"] = required unless required.empty?

        additional = body_get(body, "additional_properties")
        schema["additionalProperties"] = additional.nil? ? false : additional

        JSON.pretty_generate(schema)
      end

      private

      def body_get(body, key)
        body[key.to_sym] || body[key.to_s]
      end
    end
  end
end
