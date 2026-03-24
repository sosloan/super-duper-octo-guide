require "spec_helper"
require "genql/coordinator"

RSpec.describe Genql::Coordinator do
  subject(:coord) { described_class.default }

  # ------------------------------------------------------------------ #
  # Registration                                                         #
  # ------------------------------------------------------------------ #

  it "default coordinator registers all intent kinds" do
    expect(coord.registered_kinds.to_set).to eq(Genql::IntentKind::ALL.to_set)
  end

  it "#register returns self" do
    c = described_class.new
    expect(c.register(Genql::Compilers::SQLCompiler.new)).to be(c)
  end

  it "#use returns self" do
    c = described_class.new
    expect(c.use(->(i) { i })).to be(c)
  end

  # ------------------------------------------------------------------ #
  # Compilation — one intent per language                               #
  # ------------------------------------------------------------------ #

  it "compiles DATA to sql" do
    i   = Genql::Intent.new(Genql::IntentKind::DATA, "get_all", { table: "orders" })
    out = coord.compile(i)
    expect(out.language).to eq("sql")
    expect(out.code).to include('FROM "orders"')
  end

  it "compiles COMPUTE to python" do
    i   = Genql::Intent.new(Genql::IntentKind::COMPUTE, "square",
                            { function: "square", params: ["n"], expression: "n ** 2" })
    out = coord.compile(i)
    expect(out.language).to eq("python")
    expect(out.code).to include("def square(n):")
  end

  it "compiles SCHEMA to json_schema" do
    i   = Genql::Intent.new(Genql::IntentKind::SCHEMA, "Product",
                            { title: "Product", properties: { sku: { type: "string" } } })
    out = coord.compile(i)
    expect(out.language).to eq("json_schema")
    expect(out.code).to include("Product")
  end

  it "compiles POLICY to policy_dsl" do
    i   = Genql::Intent.new(Genql::IntentKind::POLICY, "deny_guests",
                            { subject: "guest", action: "write", resource: "any", effect: "deny" })
    out = coord.compile(i)
    expect(out.language).to eq("policy_dsl")
    expect(out.code).to include("DENY")
  end

  it "compiles WORKFLOW to workflow_dsl" do
    i   = Genql::Intent.new(Genql::IntentKind::WORKFLOW, "signup",
                            { steps: [{ name: "create_user", role: "data", action: "insert" }] })
    out = coord.compile(i)
    expect(out.language).to eq("workflow_dsl")
    expect(out.code).to include("signup")
  end

  # ------------------------------------------------------------------ #
  # compile_all                                                          #
  # ------------------------------------------------------------------ #

  it "compile_all returns one output per intent in order" do
    intents = [
      Genql::Intent.new(Genql::IntentKind::DATA,    "list", { table: "items" }),
      Genql::Intent.new(Genql::IntentKind::COMPUTE, "fn",   { function: "fn", expression: "1" })
    ]
    outputs = coord.compile_all(intents)
    expect(outputs.length).to eq(2)
    expect(outputs[0].language).to eq("sql")
    expect(outputs[1].language).to eq("python")
  end

  # ------------------------------------------------------------------ #
  # Error handling                                                       #
  # ------------------------------------------------------------------ #

  it "raises NoCompilerRegisteredError when no compiler is registered" do
    c = described_class.new
    i = Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "t" })
    expect { c.compile(i) }.to raise_error(Genql::NoCompilerRegisteredError)
  end

  # ------------------------------------------------------------------ #
  # explain                                                              #
  # ------------------------------------------------------------------ #

  it "#explain includes intent name, kind, and language" do
    i = Genql::Intent.new(Genql::IntentKind::DATA, "get_users", { table: "users" })
    explanation = coord.explain(i)
    expect(explanation).to include("get_users")
    expect(explanation).to include("data")
    expect(explanation).to include("sql")
  end

  # ------------------------------------------------------------------ #
  # Absorber integration                                                 #
  # ------------------------------------------------------------------ #

  it "absorber adds default SELECT operation for DATA intents" do
    i   = Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "products" })
    out = coord.compile(i)
    expect(out.code).to include("SELECT")
  end

  it "absorber normalises the intent name" do
    i   = Genql::Intent.new(Genql::IntentKind::DATA, "  GET_ORDERS  ", { table: "orders" })
    out = coord.compile(i)
    expect(out.intent.name).to eq("get_orders")
  end

  # ------------------------------------------------------------------ #
  # Multiple languages coexist                                           #
  # ------------------------------------------------------------------ #

  it "all five languages compile side-by-side" do
    outputs = coord.compile_all([
      Genql::Intent.new(Genql::IntentKind::DATA,     "q", { table: "t" }),
      Genql::Intent.new(Genql::IntentKind::COMPUTE,  "f", { function: "f", expression: "42" }),
      Genql::Intent.new(Genql::IntentKind::SCHEMA,   "S", { title: "S" }),
      Genql::Intent.new(Genql::IntentKind::POLICY,   "p",
                        { subject: "u", action: "r", resource: "x", effect: "allow" }),
      Genql::Intent.new(Genql::IntentKind::WORKFLOW, "w", { steps: [] })
    ])
    expect(outputs.map(&:language)).to eq(%w[sql python json_schema policy_dsl workflow_dsl])
  end
end
