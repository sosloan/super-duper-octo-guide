# genql
# ~~~~~
# GenQL — an executable theory of coordination.
#
# The system has multiple kinds of intent, and each kind gets compiled
# into the language best suited to express or enforce it.
#
# Quick-start:
#
#   require "genql"
#
#   coord = Genql::Coordinator.default
#
#   out = coord.compile(Genql::Intent.new(Genql::IntentKind::DATA, "get_users", { table: "users" }))
#   puts out.code
#   # SELECT * FROM "users";

require_relative "genql/intent"
require_relative "genql/roles"
require_relative "genql/absorber"
require_relative "genql/compilers"
require_relative "genql/coordinator"

module Genql
  VERSION = "0.1.0"
end
