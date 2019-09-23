#include "catch.hpp"
#include "flexran.pb.h"
#include "rib.h"
#include "agent_info.h"
#include <vector>

using cap = protocol::flex_bs_capability;
using spl = protocol::flex_bs_split;

flexran::rib::agent_capabilities create_caps(const std::vector<cap>& cs)
{
  protocol::flex_hello h;
  for (auto c: cs)
    h.add_capabilities(c);
  return flexran::rib::agent_capabilities(h.capabilities());
}

flexran::rib::agent_splits create_splits(const std::vector<spl>& sp)
{
  protocol::flex_hello h;
  for (auto s: sp)
    h.add_splits(s);
  return flexran::rib::agent_splits(h.splits());
}

std::shared_ptr<flexran::rib::agent_info> make_agent(
    int agent_id, uint64_t bs_id, const std::vector<cap>& cs,
    const std::vector<spl>& sp)
{
  return std::make_shared<flexran::rib::agent_info>(agent_id, bs_id, create_caps(cs), create_splits(sp), "127.0.0.1:4325");
}

TEST_CASE("RIB pending agents and add of disaggregated BS", "[rib]")
{
  flexran::rib::Rib rib;

  uint64_t bs_one = 0xe0000;
  int agent_one = 0;
  auto ai = make_agent(agent_one, bs_one, {cap::LOPHY, cap::HIPHY, cap::LOMAC}, {});

  REQUIRE (rib.add_pending_agent(ai) == true);
  REQUIRE (rib.add_pending_agent(ai) == false); // adding the same agent is forbidden
  REQUIRE (rib.get_num_pending_agents() == 1);
  REQUIRE (rib.get_available_base_stations().size() == 0);
  REQUIRE (rib.new_eNB_config_entry(bs_one) == false);
  REQUIRE (rib.has_eNB_config_entry(bs_one) == false);
  REQUIRE (rib.agent_is_pending(agent_one) == true);
  REQUIRE (rib.get_agents().size() == 0);
  REQUIRE (rib.get_bs_id(agent_one) == 0);

  SECTION ("Removal of only agent") {
    rib.remove_pending_agent(agent_one);
    REQUIRE (rib.get_num_pending_agents() == 0);
  }

  SECTION ("Second agent with the same ID and overlapping capabilities is forbidden") {
    auto aj = make_agent(3, bs_one, {cap::LOPHY}, {});
    REQUIRE (rib.add_pending_agent(aj) == false);
  }

  SECTION ("Adding second agent and complete base station") {
    int agent_two = 1;
    auto aj = make_agent(agent_two, bs_one, {cap::HIMAC, cap::RLC, cap::RRC, cap::SDAP, cap::PDCP}, {});
    REQUIRE (rib.add_pending_agent(aj) == true);
    REQUIRE (rib.get_num_pending_agents() == 2);
    REQUIRE (rib.new_eNB_config_entry(bs_one) == true);
    REQUIRE (rib.get_available_base_stations().size() == 1);
    auto bs = rib.get_bs(bs_one);
    REQUIRE (bs != nullptr);
    REQUIRE (bs->get_agents().size() == 2);
    REQUIRE (bs->get_id() == bs_one);
    REQUIRE (rib.get_bs_id(agent_one) == bs_one);
    REQUIRE (rib.get_bs_id(agent_two) == bs_one);
    REQUIRE (rib.get_num_pending_agents() == 0);
    REQUIRE (rib.get_agents().size() == 2);

    SECTION ("removal of first agent makes BS disappear") {
      REQUIRE (rib.remove_eNB_config_entry(agent_one) == true);
      REQUIRE (rib.get_available_base_stations().size() == 0);
      REQUIRE (rib.get_agents().size() == 0);
      REQUIRE (rib.get_num_pending_agents() == 1);
      rib.remove_pending_agents(bs_one);
      REQUIRE (rib.get_num_pending_agents() == 0);
    }

    SECTION ("removal of second agent makes BS disappear") {
      rib.remove_eNB_config_entry(agent_two);
      REQUIRE (rib.get_available_base_stations().size() == 0);
      REQUIRE (rib.get_num_pending_agents() == 1);
      REQUIRE (rib.get_agents().size() == 0);
    }

    SECTION ("removal of non-existing agents does not do anything") {
      REQUIRE (rib.remove_eNB_config_entry(12) == false);
      REQUIRE (rib.get_available_base_stations().size() == 1);
      REQUIRE (rib.get_num_pending_agents() == 0);
      REQUIRE (rib.get_agents().size() == 2);
    }

    SECTION ("parse agent IDs") {
      REQUIRE (rib.parse_enb_agent_id(std::to_string(agent_one)) == bs_one);
      REQUIRE (rib.parse_enb_agent_id(std::to_string(agent_two)) == bs_one);
      REQUIRE (rib.parse_enb_agent_id("2") == 0);
    }
    SECTION ("parse decimal BS IDs") {
      REQUIRE (rib.parse_enb_agent_id(std::to_string(bs_one)) == bs_one);
      REQUIRE (rib.parse_enb_agent_id("4214321") == 0);
    }
    SECTION ("parse hexadecimal BS IDs") {
      REQUIRE (rib.parse_enb_agent_id("0xe0000") == bs_one);
      REQUIRE (rib.parse_enb_agent_id("0xdeadbeef") == 0);
    }
  }

  SECTION ("adding agent of other base station makes no new base station") {
    uint64_t bs_two = 0xf0000;
    int agent_two = 3;
    auto ak = make_agent(agent_two, bs_two, {cap::HIMAC, cap::RLC, cap::RRC, cap::SDAP, cap::PDCP}, {});
    REQUIRE (rib.add_pending_agent(ak) == true);
    REQUIRE (rib.new_eNB_config_entry(bs_one) == false);
    REQUIRE (rib.new_eNB_config_entry(bs_two) == false);
    REQUIRE (rib.get_bs(bs_one) == nullptr);
    REQUIRE (rib.get_bs(bs_two) == nullptr);
    REQUIRE (rib.get_num_pending_agents() == 2);

    SECTION ("parse IDs") {
      REQUIRE (rib.parse_enb_agent_id(std::to_string(agent_one)) == 0); // BS not complete
      REQUIRE (rib.parse_enb_agent_id(std::to_string(bs_two)) == 0);; // BS not complete
    }

    SECTION ("remove all agents for BS one") {
      rib.remove_pending_agents(bs_one);
      REQUIRE (rib.get_num_pending_agents() == 1);
      REQUIRE (rib.agent_is_pending(agent_one) == false);
      REQUIRE (rib.agent_is_pending(agent_two) == true);
    }

    SECTION ("remove all agents for BS two") {
      rib.remove_pending_agents(bs_two);
      REQUIRE (rib.get_num_pending_agents() == 1);
    }
  }
}

TEST_CASE("RIB with two BS", "[rib]")
{
  flexran::rib::Rib rib;
  const std::vector<protocol::flex_bs_capability> all_caps =
      {cap::LOPHY, cap::HIPHY, cap::LOMAC, cap::HIMAC,
       cap::RLC, cap::RRC, cap::SDAP, cap::PDCP};
  const uint64_t bs1 = 0xe0000;
  const int agent1 = 0;
  const uint64_t bs2 = 0xf0000;
  const int agent2 = 2;

  // add BS1 and verify
  const auto a1 = make_agent(agent1, bs1, all_caps, {});
  REQUIRE(rib.add_pending_agent(a1) == true);
  REQUIRE(rib.new_eNB_config_entry(bs1) == true);
  REQUIRE(rib.get_available_base_stations().size() == 1);
  REQUIRE(rib.get_num_pending_agents() == 0);
  REQUIRE(rib.get_bs_from_agent(agent1) != nullptr);

  // add BS1's PHY cell configuration and verify it is set
  const int phy_cell_id1 = 10;
  protocol::flex_enb_config_reply c1;
  c1.add_cell_config()->set_phy_cell_id(phy_cell_id1);
  auto enb_rib1 = rib.get_bs_from_agent(agent1);
  enb_rib1->update_eNB_config(c1);
  REQUIRE(enb_rib1->get_enb_config().cell_config(0).has_phy_cell_id() == true);
  REQUIRE(enb_rib1->get_enb_config().cell_config(0).phy_cell_id() == phy_cell_id1);

  // add BS2 and verify we have two BS with different enb_rib_infos
  const auto a2 = make_agent(agent2, bs2, all_caps, {});
  REQUIRE(rib.add_pending_agent(a2) == true);
  REQUIRE(rib.new_eNB_config_entry(bs2) == true);
  REQUIRE(rib.get_available_base_stations().size() == 2);
  REQUIRE(rib.get_num_pending_agents() == 0);
  REQUIRE(rib.get_bs_from_agent(agent2) != nullptr);
  REQUIRE(rib.get_bs_from_agent(agent1) != rib.get_bs_from_agent(agent2));

  // add BS2's EUTRA band and verify it is set
  const int eutra_band2 = 7;
  protocol::flex_enb_config_reply c2;
  c2.add_cell_config()->set_eutra_band(eutra_band2);
  auto enb_rib2 = rib.get_bs_from_agent(agent2);
  enb_rib2->update_eNB_config(c2);
  REQUIRE(enb_rib2->get_enb_config().cell_config(0).has_eutra_band() == true);
  REQUIRE(enb_rib2->get_enb_config().cell_config(0).eutra_band() == eutra_band2);

  // check that they are still different
  enb_rib1 = rib.get_bs_from_agent(agent1);
  enb_rib2 = rib.get_bs_from_agent(agent2);
  REQUIRE(enb_rib1 != enb_rib2);

  //  and verify that BS1 has phy cell and no eutra
  //                  BS2 has no phy cell and eutra
  const auto cell_config1 = enb_rib1->get_enb_config().cell_config(0);
  const auto cell_config2 = enb_rib2->get_enb_config().cell_config(0);
  REQUIRE(cell_config1.has_phy_cell_id() == true);
  REQUIRE(cell_config1.phy_cell_id() == phy_cell_id1);
  REQUIRE(cell_config1.has_eutra_band() == false);
  REQUIRE(cell_config2.has_phy_cell_id() == false);
  REQUIRE(cell_config2.has_eutra_band() == true);
  REQUIRE(cell_config2.eutra_band() == eutra_band2);
}

TEST_CASE("Refuse BS with the same ID")
{
  flexran::rib::Rib rib;
  const std::vector<protocol::flex_bs_capability> all_caps =
      {cap::LOPHY, cap::HIPHY, cap::LOMAC, cap::HIMAC,
       cap::RLC, cap::RRC, cap::SDAP, cap::PDCP};
  const uint64_t bs1 = 0xe0000;
  const int agent1 = 0;
  const int agent2 = 2;

  // add BS1 and verify
  const auto a1 = make_agent(agent1, bs1, all_caps, {});
  REQUIRE(rib.add_pending_agent(a1) == true);
  REQUIRE(rib.new_eNB_config_entry(bs1) == true);

  // add BS2 with the same ID, must be refused
  const auto a2 = make_agent(agent2, bs1, all_caps, {});
  REQUIRE(rib.add_pending_agent(a2) == true);
  REQUIRE(rib.new_eNB_config_entry(bs1) == false);
}
