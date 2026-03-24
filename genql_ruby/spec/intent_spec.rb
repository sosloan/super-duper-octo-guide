require "spec_helper"
require "genql/intent"

RSpec.describe Genql::IntentKind do
  it "defines all five kinds" do
    expect(Genql::IntentKind::ALL).to contain_exactly(
      :data, :compute, :schema, :policy, :workflow
    )
  end
end

RSpec.describe Genql::Intent do
  let(:intent) { Genql::Intent.new(Genql::IntentKind::DATA, "get_users", { table: "users" }) }

  describe "#to_s / #inspect" do
    it "includes the kind and name" do
      expect(intent.to_s).to include("data")
      expect(intent.to_s).to include("get_users")
    end
  end

  describe "#with_metadata" do
    it "returns a copy with additional metadata, leaving original unchanged" do
      copy = intent.with_metadata(author: "alice")
      expect(copy.metadata[:author]).to eq("alice")
      expect(intent.metadata).to eq({})
    end
  end

  describe "#with_body" do
    it "returns a copy with overridden body keys, leaving original unchanged" do
      copy = intent.with_body(operation: "delete")
      expect(copy.body[:operation]).to eq("delete")
      expect(intent.body).not_to have_key(:operation)
    end
  end
end
