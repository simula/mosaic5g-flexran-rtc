#include "catch.hpp"
#include "flexran.pb.h"
#include "agent_info.h"

TEST_CASE("agent_capabilities support", "[agent_capabilities]")
{
  protocol::flex_hello hello_msg;
  hello_msg.add_capabilities(protocol::flex_bs_capability::LOMAC);
  flexran::rib::agent_capabilities caps(hello_msg.capabilities());

  REQUIRE(caps.is_complete() == false);
  REQUIRE(caps.to_string() == "[LOMAC]");
  REQUIRE(caps.size() == 1);

  SECTION("orthogonal capabilities and merging") {
    protocol::flex_hello hello_msg2;
    hello_msg2.add_capabilities(protocol::flex_bs_capability::LOPHY);
    hello_msg2.add_capabilities(protocol::flex_bs_capability::HIPHY);
    flexran::rib::agent_capabilities caps2(hello_msg2.capabilities());
    REQUIRE(caps2.size() == 2);
    REQUIRE(caps.orthogonal(caps2) == true);
    caps.merge_in(caps2);
    REQUIRE(caps.to_string() == "[LOPHY,HIPHY,LOMAC]");
    REQUIRE(caps.size() == 3);
  }

  SECTION("non-orthogonal capabilities") {
    flexran::rib::agent_capabilities caps_no(caps);
    REQUIRE(caps.orthogonal(caps_no) == false);
  }

}

TEST_CASE("agent_capabilities with completeness", "[agent_capabilities]")
{
  protocol::flex_hello hello_msg1;
  hello_msg1.add_capabilities(protocol::flex_bs_capability::LOPHY);
  hello_msg1.add_capabilities(protocol::flex_bs_capability::LOMAC);
  hello_msg1.add_capabilities(protocol::flex_bs_capability::RLC);
  hello_msg1.add_capabilities(protocol::flex_bs_capability::PDCP);
  flexran::rib::agent_capabilities caps1(hello_msg1.capabilities());
  REQUIRE(caps1.to_string() == "[LOPHY,LOMAC,RLC,PDCP]");

  protocol::flex_hello hello_msg2;
  hello_msg2.add_capabilities(protocol::flex_bs_capability::HIPHY);
  hello_msg2.add_capabilities(protocol::flex_bs_capability::HIMAC);
  hello_msg2.add_capabilities(protocol::flex_bs_capability::SDAP);
  hello_msg2.add_capabilities(protocol::flex_bs_capability::RRC);
  hello_msg2.add_capabilities(protocol::flex_bs_capability::S1AP);
  flexran::rib::agent_capabilities caps2(hello_msg2.capabilities());
  REQUIRE(caps2.to_string() == "[HIPHY,HIMAC,SDAP,RRC,S1AP]");

  REQUIRE(caps1.orthogonal(caps2) == true);
  REQUIRE(caps2.orthogonal(caps1) == true);
  caps2.merge_in(caps1);

  REQUIRE(caps2.is_complete() == true);
  REQUIRE(caps2.to_string() == "[LOPHY,HIPHY,LOMAC,HIMAC,RLC,PDCP,SDAP,RRC,S1AP]");
  REQUIRE(caps2.size() == 9);
}

TEST_CASE("agent_capabilities sorting order does not matter", "[agent_capabilities]")
{
  protocol::flex_hello hello_msg1;
  hello_msg1.add_capabilities(protocol::flex_bs_capability::LOPHY);
  hello_msg1.add_capabilities(protocol::flex_bs_capability::HIMAC);
  flexran::rib::agent_capabilities caps1(hello_msg1.capabilities());

  protocol::flex_hello hello_msg2;
  hello_msg2.add_capabilities(protocol::flex_bs_capability::HIMAC);
  hello_msg2.add_capabilities(protocol::flex_bs_capability::LOPHY);
  flexran::rib::agent_capabilities caps2(hello_msg2.capabilities());

  REQUIRE(caps1.to_string() == caps2.to_string());
}
