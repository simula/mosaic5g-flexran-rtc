#include "catch.hpp"
#include "flexran.pb.h"
#include "rib.h"
#include "agent_info.h"
#include <vector>
#include <chrono>

using cap = protocol::flex_bs_capability;

flexran::rib::agent_capabilities create_caps(const std::vector<cap>& cs)
{
  protocol::flex_hello h;
  for (auto c: cs)
    h.add_capabilities(c);
  return flexran::rib::agent_capabilities(h.capabilities());
}

std::shared_ptr<flexran::rib::agent_info> make_agent(
    int agent_id, uint64_t bs_id, const std::vector<cap>& cs)
{
  return std::make_shared<flexran::rib::agent_info>(
    agent_id, bs_id, create_caps(cs), std::chrono::microseconds(0));
}

TEST_CASE("RIB pending agents and add of disaggregated BS", "[rib]")
{
  flexran::rib::Rib rib;

  uint64_t bs_one = 0xe0000;
  int agent_one = 0;
  auto ai = make_agent(agent_one, bs_one, {cap::LOPHY, cap::HIPHY, cap::LOMAC});

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

  SECTION ("Adding second agent and complete base station") {
    int agent_two = 1;
    auto aj = make_agent(agent_two, bs_one, {cap::HIMAC, cap::RLC, cap::RRC, cap::SDAP, cap::PDCP});
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
    auto ak = make_agent(agent_two, bs_two, {cap::HIMAC, cap::RLC, cap::RRC, cap::SDAP, cap::PDCP});
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
