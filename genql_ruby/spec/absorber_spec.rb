require "spec_helper"
require "genql/absorber"
require "genql/compilers/sql_compiler"

RSpec.describe Genql::Absorber do
  let(:data_intent) { Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "t" }) }

  it "returns the intent unchanged when the chain is empty" do
    absorber = described_class.new
    expect(absorber.absorb(data_intent)).to be(data_intent)
  end

  it "executes middlewares in registration order" do
    log = []
    absorber = described_class.new
    absorber.use(->(i) { log << "first"; i })
    absorber.use(->(i) { log << "second"; i })
    absorber.absorb(data_intent)
    expect(log).to eq(%w[first second])
  end

  it "#use returns self for chaining" do
    absorber = described_class.new
    expect(absorber.use(->(i) { i })).to be(absorber)
  end

  describe "NORMALISE_NAME" do
    it "lowercases and strips the intent name" do
      i = Genql::Intent.new(Genql::IntentKind::DATA, "  GET_USERS  ", { table: "users" })
      result = Genql::Absorber::NORMALISE_NAME.call(i)
      expect(result.name).to eq("get_users")
    end

    it "returns the original object when the name is already clean" do
      i = Genql::Intent.new(Genql::IntentKind::DATA, "get_users", { table: "users" })
      expect(Genql::Absorber::NORMALISE_NAME.call(i)).to be(i)
    end
  end

  describe "DEFAULT_OPERATION" do
    it "adds operation: select for DATA intents that omit it" do
      i = Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "users" })
      result = Genql::Absorber::DEFAULT_OPERATION.call(i)
      expect(result.body[:operation]).to eq("select")
    end

    it "does not override an existing operation" do
      i = Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "users", operation: "delete" })
      result = Genql::Absorber::DEFAULT_OPERATION.call(i)
      expect(result.body[:operation]).to eq("delete")
    end

    it "ignores non-DATA intents" do
      i = Genql::Intent.new(Genql::IntentKind::COMPUTE, "fn", { function: "f" })
      expect(Genql::Absorber::DEFAULT_OPERATION.call(i)).to be(i)
    end
  end

  describe "REQUIRE_TABLE" do
    it "passes through DATA intents that have a table" do
      i = Genql::Intent.new(Genql::IntentKind::DATA, "q", { table: "users" })
      expect(Genql::Absorber::REQUIRE_TABLE.call(i)).to be(i)
    end

    it "raises for DATA intents missing table" do
      i = Genql::Intent.new(Genql::IntentKind::DATA, "q", {})
      expect { Genql::Absorber::REQUIRE_TABLE.call(i) }.to raise_error(ArgumentError, /table/)
    end

    it "ignores non-DATA intents" do
      i = Genql::Intent.new(Genql::IntentKind::COMPUTE, "fn", {})
      expect(Genql::Absorber::REQUIRE_TABLE.call(i)).to be(i)
    end
  end

  it "full pipeline: normalise + default_operation + require_table + SQL compile" do
    absorber = Genql::Absorber.new
      .use(Genql::Absorber::NORMALISE_NAME)
      .use(Genql::Absorber::DEFAULT_OPERATION)
      .use(Genql::Absorber::REQUIRE_TABLE)

    i = Genql::Intent.new(Genql::IntentKind::DATA, "  GET_USERS  ", { table: "users" })
    processed = absorber.absorb(i)
    expect(processed.name).to eq("get_users")
    expect(processed.body[:operation]).to eq("select")

    sql = Genql::Compilers::SQLCompiler.new.compile(processed)
    expect(sql).to eq('SELECT * FROM "users";')
  end
end
