require "spec_helper"
require "json"
require "genql/compilers"
require "genql/intent"

# ------------------------------------------------------------------ #
# SQLCompiler                                                          #
# ------------------------------------------------------------------ #

RSpec.describe Genql::Compilers::SQLCompiler do
  subject(:compiler) { described_class.new }

  it "supports DATA kind" do
    expect(compiler.supported_kinds).to include(Genql::IntentKind::DATA)
  end

  it "has target_language sql" do
    expect(compiler.target_language).to eq("sql")
  end

  it "SELECT *" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "users", operation: "select" })
    expect(compiler.compile(i)).to eq('SELECT * FROM "users";')
  end

  it "SELECT specific columns" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q",
                          { table: "users", operation: "select", columns: %w[id name] })
    expect(compiler.compile(i)).to eq('SELECT "id", "name" FROM "users";')
  end

  it "SELECT with WHERE integer" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q",
                          { table: "users", operation: "select", where: { id: 1 } })
    expect(compiler.compile(i)).to eq('SELECT * FROM "users" WHERE "id" = 1;')
  end

  it "SELECT with WHERE string (quoted)" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q",
                          { table: "users", operation: "select", where: { name: "alice" } })
    expect(compiler.compile(i)).to include('"name" = \'alice\'')
  end

  it "INSERT" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q",
                          { table: "users", operation: "insert", values: { name: "bob", age: 30 } })
    sql = compiler.compile(i)
    expect(sql).to start_with('INSERT INTO "users"')
    expect(sql).to include("'bob'")
    expect(sql).to include("30")
  end

  it "UPDATE" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q",
                          { table: "users", operation: "update",
                            values: { name: "carol" }, where: { id: 5 } })
    sql = compiler.compile(i)
    expect(sql).to include("UPDATE")
    expect(sql).to include("'carol'")
    expect(sql).to include('"id" = 5')
  end

  it "DELETE" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q",
                          { table: "users", operation: "delete", where: { id: 5 } })
    expect(compiler.compile(i)).to eq('DELETE FROM "users" WHERE "id" = 5;')
  end

  it "raises for unknown operation" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "t", operation: "drop" })
    expect { compiler.compile(i) }.to raise_error(ArgumentError, /unknown operation/)
  end

  it "escapes single quotes in string literals" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q",
                          { table: "t", operation: "select", where: { name: "O'Brien" } })
    expect(compiler.compile(i)).to include("O''Brien")
  end
end

# ------------------------------------------------------------------ #
# PythonCompiler                                                       #
# ------------------------------------------------------------------ #

RSpec.describe Genql::Compilers::PythonCompiler do
  subject(:compiler) { described_class.new }

  it "supports COMPUTE kind" do
    expect(compiler.supported_kinds).to include(Genql::IntentKind::COMPUTE)
  end

  it "compiles an expression-based function" do
    i = Genql::Intent.new(Genql::IntentKind::COMPUTE, "double",
                          { function: "double", params: ["x"], expression: "x * 2" })
    code = compiler.compile(i)
    expect(code).to include("def double(x):")
    expect(code).to include("return x * 2")
  end

  it "compiles a steps-based function" do
    i = Genql::Intent.new(Genql::IntentKind::COMPUTE, "greet",
                          { function: "greet", params: ["name"],
                            steps: ['msg = f"Hello, {name}"', "return msg"] })
    code = compiler.compile(i)
    expect(code).to include("def greet(name):")
    expect(code).to include("return msg")
  end

  it "includes return type annotation" do
    i = Genql::Intent.new(Genql::IntentKind::COMPUTE, "add",
                          { function: "add", params: %w[a b], expression: "a + b", returns: "int" })
    expect(compiler.compile(i)).to include("-> int:")
  end

  it "generates pass for an empty body" do
    i = Genql::Intent.new(Genql::IntentKind::COMPUTE, "noop", {})
    expect(compiler.compile(i)).to include("pass")
  end
end

# ------------------------------------------------------------------ #
# SchemaCompiler                                                       #
# ------------------------------------------------------------------ #

RSpec.describe Genql::Compilers::SchemaCompiler do
  subject(:compiler) { described_class.new }

  it "supports SCHEMA kind" do
    expect(compiler.supported_kinds).to include(Genql::IntentKind::SCHEMA)
  end

  it "compiles a basic schema" do
    i = Genql::Intent.new(Genql::IntentKind::SCHEMA, "User",
                          { title: "User",
                            properties: { id: { type: "integer" }, name: { type: "string" } },
                            required: %w[id name] })
    schema = JSON.parse(compiler.compile(i))
    expect(schema["title"]).to eq("User")
    expect(schema["type"]).to eq("object")
    expect(schema["required"]).to eq(%w[id name])
    expect(schema["additionalProperties"]).to be false
  end

  it "defaults additionalProperties to false" do
    i = Genql::Intent.new(Genql::IntentKind::SCHEMA, "T", { title: "T" })
    schema = JSON.parse(compiler.compile(i))
    expect(schema["additionalProperties"]).to be false
  end

  it "includes the 2020-12 JSON Schema draft reference" do
    i = Genql::Intent.new(Genql::IntentKind::SCHEMA, "T", {})
    schema = JSON.parse(compiler.compile(i))
    expect(schema["$schema"]).to include("2020-12")
  end
end

# ------------------------------------------------------------------ #
# PolicyCompiler                                                       #
# ------------------------------------------------------------------ #

RSpec.describe Genql::Compilers::PolicyCompiler do
  subject(:compiler) { described_class.new }

  it "supports POLICY kind" do
    expect(compiler.supported_kinds).to include(Genql::IntentKind::POLICY)
  end

  it "compiles an allow policy" do
    i = Genql::Intent.new(Genql::IntentKind::POLICY, "admin_read",
                          { subject: "admin", action: "read",
                            resource: "document", effect: "allow" })
    dsl = compiler.compile(i)
    expect(dsl).to include("EFFECT   ALLOW")
    expect(dsl).to include("SUBJECT  admin")
    expect(dsl).to include("RESOURCE document")
  end

  it "includes conditions" do
    i = Genql::Intent.new(Genql::IntentKind::POLICY, "owner_only",
                          { subject: "user", action: "write", resource: "file",
                            effect: "allow", conditions: ["user.id == file.owner_id"] })
    expect(compiler.compile(i)).to include("user.id == file.owner_id")
  end

  it "includes priority" do
    i = Genql::Intent.new(Genql::IntentKind::POLICY, "high_pri",
                          { subject: "s", action: "a", resource: "r",
                            effect: "deny", priority: 10 })
    expect(compiler.compile(i)).to include("priority=10")
  end
end

# ------------------------------------------------------------------ #
# WorkflowCompiler                                                     #
# ------------------------------------------------------------------ #

RSpec.describe Genql::Compilers::WorkflowCompiler do
  subject(:compiler) { described_class.new }

  it "supports WORKFLOW kind" do
    expect(compiler.supported_kinds).to include(Genql::IntentKind::WORKFLOW)
  end

  it "compiles a workflow with steps and dependencies" do
    i = Genql::Intent.new(Genql::IntentKind::WORKFLOW, "onboard_user",
                          { steps: [
                            { name: "validate", role: "schema", action: "validate_input" },
                            { name: "persist",  role: "data",   action: "insert_user",
                              depends_on: ["validate"] }
                          ] })
    dsl = compiler.compile(i)
    expect(dsl).to include("WORKFLOW 'onboard_user'")
    expect(dsl).to include("role:       schema")
    expect(dsl).to include("depends_on: [validate]")
  end

  it "handles empty steps" do
    i = Genql::Intent.new(Genql::IntentKind::WORKFLOW, "empty", { steps: [] })
    dsl = compiler.compile(i)
    expect(dsl).to include("WORKFLOW 'empty'")
  end
end
