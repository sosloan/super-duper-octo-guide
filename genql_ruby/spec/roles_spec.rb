require "spec_helper"
require "genql/roles"

RSpec.describe "Genql roles" do
  describe "ALL_ROLES" do
    it "covers all intent kinds" do
      covered = Genql::ALL_ROLES.flat_map(&:owned_kinds).to_set
      expect(covered).to eq(Genql::IntentKind::ALL.to_set)
    end

    it "has distinct target languages" do
      languages = Genql::ALL_ROLES.map(&:target_language)
      expect(languages.length).to eq(languages.uniq.length)
    end
  end

  describe "Genql.role_for" do
    it "returns the correct role for each kind" do
      expect(Genql.role_for(Genql::IntentKind::DATA)).to be(Genql::DATA_ROLE)
      expect(Genql.role_for(Genql::IntentKind::COMPUTE)).to be(Genql::COMPUTE_ROLE)
      expect(Genql.role_for(Genql::IntentKind::SCHEMA)).to be(Genql::SCHEMA_ROLE)
      expect(Genql.role_for(Genql::IntentKind::POLICY)).to be(Genql::POLICY_ROLE)
      expect(Genql.role_for(Genql::IntentKind::WORKFLOW)).to be(Genql::WORKFLOW_ROLE)
    end
  end

  describe Genql::Role do
    it "#owns? returns true for owned kinds and false otherwise" do
      expect(Genql::DATA_ROLE.owns?(Genql::IntentKind::DATA)).to be true
      expect(Genql::DATA_ROLE.owns?(Genql::IntentKind::COMPUTE)).to be false
    end
  end
end
