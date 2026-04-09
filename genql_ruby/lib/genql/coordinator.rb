# genql/coordinator
#
# The coordination engine — the heart of GenQL.
#
# The Coordinator is the place where *languages are made to coexist* and
# *roles are separated*.  It:
#
# 1. Holds a registry of Compiler instances, each responsible for one or
#    more IntentKind values.
# 2. Routes each Intent to the compiler that can handle it.
# 3. Passes every intent through an Absorber *before* compilation.
# 4. Provides a single #compile / #compile_all surface to callers.

require_relative "absorber"
require_relative "compilers"
require_relative "intent"
require_relative "roles"

module Genql
  class NoCompilerRegisteredError < KeyError; end

  class Coordinator
    def initialize(absorber: nil)
      @compilers = {}
      @absorber  = absorber || Absorber.new
    end

    # ------------------------------------------------------------------ #
    # Registration                                                         #
    # ------------------------------------------------------------------ #

    # Register +compiler+ for every kind it supports. Returns self.
    def register(compiler)
      compiler.supported_kinds.each do |kind|
        @compilers[kind] = compiler
      end
      self
    end

    # Add a middleware to the absorber chain. Returns self.
    def use(middleware)
      @absorber.use(middleware)
      self
    end

    # ------------------------------------------------------------------ #
    # Compilation                                                          #
    # ------------------------------------------------------------------ #

    # Absorb and compile a single +intent+.
    #
    # @raise [NoCompilerRegisteredError] if no compiler handles the intent kind.
    def compile(intent)
      intent   = @absorber.absorb(intent)
      compiler = @compilers[intent.kind]

      unless compiler
        raise NoCompilerRegisteredError,
              "No compiler registered for intent kind #{intent.kind.inspect}. " \
              "Register one with coordinator.register(compiler)."
      end

      code = compiler.compile(intent)
      Compilers::CompiledOutput.new(intent: intent, language: compiler.target_language, code: code)
    end

    # Absorb and compile every intent in +intents+, in order.
    def compile_all(intents)
      intents.map { |i| compile(i) }
    end

    # ------------------------------------------------------------------ #
    # Introspection                                                        #
    # ------------------------------------------------------------------ #

    def registered_kinds
      @compilers.keys
    end

    def role_for(kind)
      Genql.role_for(kind)
    end

    def explain(intent)
      role     = Genql.role_for(intent.kind) rescue nil
      compiler = @compilers[intent.kind]
      compiler_name = compiler ? compiler.class.name.split("::").last : "(unregistered)"

      if role
        "Intent #{intent.name.inspect} [#{intent.kind}]\n" \
        "  Role:     #{role.name} — #{role.description}\n" \
        "  Compiler: #{compiler_name}\n" \
        "  Language: #{role.target_language}"
      else
        "Intent #{intent.name.inspect} [#{intent.kind}] — no role defined"
      end
    end

    # ------------------------------------------------------------------ #
    # Factory                                                              #
    # ------------------------------------------------------------------ #

    # Return a Coordinator pre-loaded with all built-in compilers and the
    # standard absorber middlewares.
    def self.default
      absorber = Absorber.new
        .use(Absorber::NORMALISE_NAME)
        .use(Absorber::DEFAULT_OPERATION)
        .use(Absorber::REQUIRE_TABLE)

      new(absorber: absorber)
        .register(Compilers::SQLCompiler.new)
        .register(Compilers::PythonCompiler.new)
        .register(Compilers::SchemaCompiler.new)
        .register(Compilers::PolicyCompiler.new)
        .register(Compilers::WorkflowCompiler.new)
    end
  end
end
