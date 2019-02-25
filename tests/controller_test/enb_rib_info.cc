#include "catch.hpp"
#include "flexran.pb.h"
#include "enb_rib_info.h"
#include <iostream>

TEST_CASE("test update_eNB_config", "[enb_rib_info]")
{
  // The following tests that the protobuf messages are correctly merged in the
  // RIB.  This is because parts of messages come from different agents.

  const int phy_cell_id1 = 10;
  const int phy_cell_id2 = 20;
  const int eutra_band = 7;

  flexran::rib::enb_rib_info rib_info(1, {}); // no agents (doesn't matter)

  protocol::flex_enb_config_reply c1;
  c1.add_cell_config()->set_phy_cell_id(phy_cell_id1);
  rib_info.update_eNB_config(c1);
  REQUIRE (rib_info.get_enb_config().cell_config(0).has_phy_cell_id() == true);
  REQUIRE (rib_info.get_enb_config().cell_config(0).phy_cell_id() == phy_cell_id1);
  REQUIRE (rib_info.get_enb_config().cell_config(0).has_eutra_band() == false);

  protocol::flex_enb_config_reply c2;
  c2.add_cell_config()->set_eutra_band(eutra_band);
  rib_info.update_eNB_config(c2);
  REQUIRE (rib_info.get_enb_config().cell_config_size() == 1);
  REQUIRE (rib_info.get_enb_config().cell_config(0).has_phy_cell_id() == true);
  REQUIRE (rib_info.get_enb_config().cell_config(0).phy_cell_id() == phy_cell_id1);
  REQUIRE (rib_info.get_enb_config().cell_config(0).has_eutra_band() == true);
  REQUIRE (rib_info.get_enb_config().cell_config(0).eutra_band() == eutra_band);

  c1.mutable_cell_config(0)->set_phy_cell_id(phy_cell_id2);
  rib_info.update_eNB_config(c1);
  REQUIRE (rib_info.get_enb_config().cell_config(0).has_phy_cell_id() == true);
  REQUIRE (rib_info.get_enb_config().cell_config(0).phy_cell_id() == phy_cell_id2);
  REQUIRE (rib_info.get_enb_config().cell_config(0).has_eutra_band() == true);
  REQUIRE (rib_info.get_enb_config().cell_config(0).eutra_band() == eutra_band);
}

TEST_CASE("test update_eNB_config with repeated field inside flex_cell_config", "[enb_rib_info]")
{
  const int i1 = 1440, i2 = 22, i3 = 1337;
  flexran::rib::enb_rib_info rib_info(1, {});

  protocol::flex_enb_config_reply c1;
  c1.add_cell_config()->add_mbsfn_subframe_config_rfperiod(i1);
  rib_info.update_eNB_config(c1);
  REQUIRE (rib_info.get_enb_config().cell_config_size() == 1);
  REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod_size() == 1);
  REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod(0) == i1);

  SECTION("rewrite the same field from different messages") {
    protocol::flex_enb_config_reply c2;
    c2.add_cell_config()->add_mbsfn_subframe_config_rfperiod(i2);
    rib_info.update_eNB_config(c2);
    REQUIRE (rib_info.get_enb_config().cell_config_size() == 1);
    REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod_size() == 1);
    REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod(0) == i2);

    c1.mutable_cell_config(0)->add_mbsfn_subframe_config_rfperiod(i3);
    rib_info.update_eNB_config(c1);
    REQUIRE (rib_info.get_enb_config().cell_config_size() == 1);
    REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod_size() == 2);
    REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod(0) == i1);
    REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod(1) == i3);
  }

  SECTION("write to another field, should preserve both") {
    protocol::flex_enb_config_reply c3;
    c3.add_cell_config()->set_phy_cell_id(i3);
    rib_info.update_eNB_config(c3);
    REQUIRE (rib_info.get_enb_config().cell_config_size() == 1);
    REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod_size() == 1);
    REQUIRE (rib_info.get_enb_config().cell_config(0).mbsfn_subframe_config_rfperiod(0) == i1);
    REQUIRE (rib_info.get_enb_config().cell_config(0).phy_cell_id() == i3);
  }
}

TEST_CASE("test update_UE_config for various scenarios of ue_state_change and ue_config_reply", "[enb_rib_info]")
{
  const int rnti1 = 0x1234;
  const int rnti2 = 0xabcd; // test assumes that rnti1 < rnti2
  const int rnti_n = 0x8531;
  const uint64_t imsi = 12345567890;
  const int dl_sid = 132;
  const int ul_sid = 231;
  flexran::rib::enb_rib_info rib_info(1, {}); // no agents (doesn't matter)

  protocol::flex_ue_state_change sc1;
  sc1.set_type(protocol::FLUESC_ACTIVATED);
  sc1.mutable_config()->set_rnti(rnti1);
  rib_info.update_UE_config(sc1);
  REQUIRE (rib_info.get_ue_configs().ue_config_size() == 1);
  REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
  REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
  REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == false);


  SECTION("state change \"activate\" with the same message") {
    // activating again with same message preserves everything (does not add,
    // must be unique)
    rib_info.update_UE_config(sc1);
    REQUIRE (rib_info.get_ue_configs().ue_config_size() == 1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == false);
  }

  SECTION("state change \"activate\" with different message") {
    // activating again with different message effectively updates
    protocol::flex_ue_state_change sc2;
    sc2.set_type(protocol::FLUESC_ACTIVATED);
    sc2.mutable_config()->set_rnti(rnti1);
    sc2.mutable_config()->set_imsi(imsi);
    rib_info.update_UE_config(sc2);
    REQUIRE (rib_info.get_ue_configs().ue_config_size() == 1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).imsi() == imsi);
  }

  SECTION("state change \"update\" with the same message") {
    sc1.set_type(protocol::FLUESC_UPDATED);
    rib_info.update_UE_config(sc1);
    REQUIRE (rib_info.get_ue_configs().ue_config_size() == 1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == false);
  }

  SECTION("state change \"update\" with different message") {
    // update with different message parts, updates mentioned
    protocol::flex_ue_state_change sc2;
    sc2.set_type(protocol::FLUESC_UPDATED);
    sc2.mutable_config()->set_rnti(rnti1);
    sc2.mutable_config()->set_imsi(imsi);
    rib_info.update_UE_config(sc2);
    REQUIRE (rib_info.get_ue_configs().ue_config_size() == 1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).imsi() == imsi);
  }

  SECTION("update UE config") {
    // update config, replaces first config for changed parts
    protocol::flex_ue_config_reply cr1;
    cr1.add_ue_config()->set_rnti(rnti1);
    cr1.mutable_ue_config(0)->set_imsi(imsi);
    rib_info.update_UE_config(cr1);
    REQUIRE (rib_info.get_ue_configs().ue_config_size() == 1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == true);
    REQUIRE (rib_info.get_ue_configs().ue_config(0).imsi() == imsi);

    SECTION("state change \"activate\" for 2nd UE") {
      // activate state change 2nd UE, preserves first + second
      protocol::flex_ue_state_change sc2;
      sc2.set_type(protocol::FLUESC_ACTIVATED);
      sc2.mutable_config()->set_rnti(rnti2);
      rib_info.update_UE_config(sc2);
      REQUIRE (rib_info.get_ue_configs().ue_config_size() == 2);
      REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
      REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
      REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == true);
      REQUIRE (rib_info.get_ue_configs().ue_config(0).imsi() == imsi);
      REQUIRE (rib_info.get_ue_configs().ue_config(1).has_rnti() == true);
      REQUIRE (rib_info.get_ue_configs().ue_config(1).rnti() == rnti2);

      SECTION("state change \"update\" of 2nd UE") {
        // update state change 2nd UE
        sc2.set_type(protocol::FLUESC_UPDATED);
        sc2.mutable_config()->set_dl_slice_id(dl_sid);
        rib_info.update_UE_config(sc2);
        REQUIRE (rib_info.get_ue_configs().ue_config_size() == 2);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).rnti() == rnti2);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_imsi() == false);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_dl_slice_id() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).dl_slice_id() == dl_sid);
      }

      SECTION("ue_config_reply of 2nd UE is like its update") {
        protocol::flex_ue_config_reply cr2;
        cr2.add_ue_config()->set_rnti(rnti1);
        cr2.mutable_ue_config(0)->set_ul_slice_id(ul_sid);
        cr2.add_ue_config()->set_rnti(rnti2);
        cr2.mutable_ue_config(1)->set_dl_slice_id(dl_sid);
        rib_info.update_UE_config(cr2);
        REQUIRE (rib_info.get_ue_configs().ue_config_size() == 2);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).imsi() == imsi);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_ul_slice_id() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).ul_slice_id() == ul_sid);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).rnti() == rnti2);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_imsi() == false);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_dl_slice_id() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).dl_slice_id() == dl_sid);
      }

      SECTION("ue_config_reply with UEs in wrong order (by RNTI) is applied") {
        protocol::flex_ue_config_reply cr2;
        cr2.add_ue_config()->set_rnti(rnti2);
        cr2.mutable_ue_config(0)->set_dl_slice_id(dl_sid);
        cr2.add_ue_config()->set_rnti(rnti1);
        cr2.mutable_ue_config(1)->set_ul_slice_id(ul_sid);
        rib_info.update_UE_config(cr2);
        REQUIRE (rib_info.get_ue_configs().ue_config_size() == 2);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).imsi() == imsi);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_ul_slice_id() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).ul_slice_id() == ul_sid);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).rnti() == rnti2);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_imsi() == false);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_dl_slice_id() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).dl_slice_id() == dl_sid);
      }

      SECTION("state change \"update\" with wrong rnti does nothing") {
        protocol::flex_ue_config_reply cr2;
        cr2.add_ue_config()->set_rnti(rnti_n);
        cr2.mutable_ue_config(0)->set_ul_slice_id(ul_sid);
        rib_info.update_UE_config(cr2);
        REQUIRE (rib_info.get_ue_configs().ue_config_size() == 2);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).imsi() == imsi);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_ul_slice_id() == false);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).rnti() == rnti2);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_imsi() == false);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_ul_slice_id() == false);
      }

      SECTION("state change \"deactivate\" of 1st UE") {
        protocol::flex_ue_state_change scd;
        scd.set_type(protocol::FLUESC_DEACTIVATED);
        scd.mutable_config()->set_rnti(rnti1);
        rib_info.update_UE_config(scd);
        REQUIRE (rib_info.get_ue_configs().ue_config_size() == 1);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti2);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_imsi() == false);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_ul_slice_id() == false);
      }

      SECTION("state change \"deactivate\" with wrong RNTI does nothing") {
        protocol::flex_ue_state_change scd;
        scd.set_type(protocol::FLUESC_DEACTIVATED);
        scd.mutable_config()->set_rnti(rnti_n);
        rib_info.update_UE_config(scd);
        REQUIRE (rib_info.get_ue_configs().ue_config_size() == 2);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(0).rnti() == rnti1);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).has_rnti() == true);
        REQUIRE (rib_info.get_ue_configs().ue_config(1).rnti() == rnti2);
      }
    }
  }
}

TEST_CASE("test update_LC_config correct merging", "[enb_rib_info]")
{
  const int rnti1 = 0x1337, rnti2 = 0x2446;
  const int lcid1 = 3, lcid2 = 4;

  flexran::rib::enb_rib_info rib_info(1, {});

  protocol::flex_lc_config_reply c1;
  protocol::flex_lc_ue_config *uec = c1.add_lc_ue_config();
  uec->set_rnti(rnti1);
  uec->add_lc_config()->set_lcid(lcid1);
  rib_info.update_LC_config(c1);

  REQUIRE (rib_info.get_lc_configs().lc_ue_config_size() == 1);
  REQUIRE (rib_info.get_lc_configs().lc_ue_config(0).rnti() == rnti1);
  REQUIRE (rib_info.get_lc_configs().lc_ue_config(0).lc_config_size() == 1);
  REQUIRE (rib_info.get_lc_configs().lc_ue_config(0).lc_config(0).lcid() == lcid1);

  protocol::flex_lc_config_reply c2;
  uec = c2.add_lc_ue_config();
  uec->set_rnti(rnti2);
  uec->add_lc_config()->set_lcid(lcid2);
  rib_info.update_LC_config(c2);

  REQUIRE (rib_info.get_lc_configs().lc_ue_config_size() == 1);
  REQUIRE (rib_info.get_lc_configs().lc_ue_config(0).rnti() == rnti2);
  REQUIRE (rib_info.get_lc_configs().lc_ue_config(0).lc_config_size() == 1);
  REQUIRE (rib_info.get_lc_configs().lc_ue_config(0).lc_config(0).lcid() == lcid2);
}
