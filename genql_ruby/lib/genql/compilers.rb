# genql/compilers
#
# Convenience loader for all built-in compilers.

require_relative "compilers/base"
require_relative "compilers/sql_compiler"
require_relative "compilers/python_compiler"
require_relative "compilers/schema_compiler"
require_relative "compilers/policy_compiler"
require_relative "compilers/workflow_compiler"
