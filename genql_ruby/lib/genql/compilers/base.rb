# genql/compilers/base
#
# Abstract base class shared by all language-specific compilers.

require_relative "../intent"

module Genql
  module Compilers
    # The result of compiling a single Intent.
    class CompiledOutput
      attr_reader :intent, :language, :code

      REPR_PREVIEW_LEN = 60

      def initialize(intent:, language:, code:)
        @intent   = intent
        @language = language
        @code     = code
      end

      def to_s
        preview = @code[0, REPR_PREVIEW_LEN].gsub("\n", " ")
        "CompiledOutput(language=#{@language.inspect}, code=#{preview.inspect}…)"
      end

      alias inspect to_s
    end

    # Abstract base for all GenQL compilers.
    #
    # Subclasses must implement:
    #   - target_language  → String
    #   - supported_kinds  → Array<Symbol>
    #   - compile(intent)  → String
    class Compiler
      def target_language
        raise NotImplementedError, "#{self.class}#target_language not implemented"
      end

      def supported_kinds
        raise NotImplementedError, "#{self.class}#supported_kinds not implemented"
      end

      def compile(intent)
        raise NotImplementedError, "#{self.class}#compile not implemented"
      end

      def can_compile?(intent)
        supported_kinds.include?(intent.kind)
      end
    end
  end
end
