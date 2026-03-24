# genql/compilers/sql_compiler
#
# Compiles DATA intents into SQL statements.
#
# Supported body keys
# -------------------
# operation : String  — "select" | "insert" | "update" | "delete"
# table     : String  — target table name
# columns   : Array   — columns to retrieve (select only); omit or ["*"] for all
# where     : Hash    — equality conditions, e.g. {id: 42}
# values    : Hash    — column → value mapping (insert / update)

require_relative "base"
require_relative "../intent"

module Genql
  module Compilers
    class SQLCompiler < Compiler
      def target_language
        "sql"
      end

      def supported_kinds
        [IntentKind::DATA]
      end

      def compile(intent)
        op = (body_get(intent.body, "operation") || "select").to_s.downcase
        case op
        when "select" then sql_select(intent.body)
        when "insert" then sql_insert(intent.body)
        when "update" then sql_update(intent.body)
        when "delete" then sql_delete(intent.body)
        else
          raise ArgumentError,
                "SQLCompiler: unknown operation #{op.inspect}. " \
                "Expected one of [\"select\", \"insert\", \"update\", \"delete\"]."
        end
      end

      private

      # Accept both string and symbol keys
      def body_get(body, key)
        body[key.to_sym] || body[key.to_s]
      end

      def quote(identifier)
        "\"#{identifier}\""
      end

      def literal(value)
        case value
        when String then "'#{value.gsub("'", "''")}'"
        when NilClass then "NULL"
        else value.to_s
        end
      end

      def where_clause(where)
        return "" if where.nil? || where.empty?

        parts = where.map { |col, val| "#{quote(col)} = #{literal(val)}" }
        " WHERE #{parts.join(' AND ')}"
      end

      def sql_select(body)
        table   = quote(body_get(body, "table"))
        cols    = body_get(body, "columns") || ["*"]
        col_str = cols.map { |c| c == "*" ? "*" : quote(c) }.join(", ")
        where   = where_clause(body_get(body, "where") || {})
        "SELECT #{col_str} FROM #{table}#{where};"
      end

      def sql_insert(body)
        table  = quote(body_get(body, "table"))
        values = body_get(body, "values") || {}
        cols   = values.keys.map { |c| quote(c) }.join(", ")
        vals   = values.values.map { |v| literal(v) }.join(", ")
        "INSERT INTO #{table} (#{cols}) VALUES (#{vals});"
      end

      def sql_update(body)
        table  = quote(body_get(body, "table"))
        values = body_get(body, "values") || {}
        set    = values.map { |c, v| "#{quote(c)} = #{literal(v)}" }.join(", ")
        where  = where_clause(body_get(body, "where") || {})
        "UPDATE #{table} SET #{set}#{where};"
      end

      def sql_delete(body)
        table = quote(body_get(body, "table"))
        where = where_clause(body_get(body, "where") || {})
        "DELETE FROM #{table}#{where};"
      end
    end
  end
end
