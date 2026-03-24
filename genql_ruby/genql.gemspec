Gem::Specification.new do |spec|
  spec.name          = "genql"
  spec.version       = "0.1.0"
  spec.summary       = "GenQL — an executable theory of coordination (Ruby backend)"
  spec.description   = "Routes typed intents to language-specific compilers " \
                       "(SQL, Python, JSON Schema, Policy DSL, Workflow DSL)."
  spec.authors       = ["GenQL Contributors"]
  spec.files         = Dir["lib/**/*.rb"]
  spec.require_paths = ["lib"]
  spec.required_ruby_version = ">= 3.0"
end
