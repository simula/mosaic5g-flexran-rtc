#include "catch.hpp"
#include "flexible_scheduler.h"
#include <vector>
#include <iostream>

using sched = flexran::app::scheduler::flexible_scheduler;

TEST_CASE("test correct parsing of IMSIs", "[flexible_scheduler]")
{
  std::vector<uint64_t> imsis;
  bool ret;
  std::string err;

  std::string list = "";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == false);
  REQUIRE(imsis.size() == 0);

  list = "123";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == false);
  REQUIRE(imsis.size() == 0);

  list = "  [   ]   ";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == true);
  REQUIRE(imsis.size() == 0);

  list = "[124]";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == true);
  REQUIRE(imsis.size() == 1);
  REQUIRE(imsis.at(0) == 124);

  imsis.clear();
  list = "[\n\n\n    \t123, \n      123451]";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == true);
  REQUIRE(imsis.size() == 2);
  REQUIRE(imsis.at(0) == 123);
  REQUIRE(imsis.at(1) == 123451);

  imsis.clear();
  list = "[\n\n\n    \t123, \n      123451a]";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == false);
  REQUIRE(imsis.size() == 0);

  list = "[\n\n\n    \t123, a\n      123451]";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == false);
  REQUIRE(imsis.size() == 0);

  list = "[\n\n\n    \t123, \n      123451,";
  ret = sched::parse_imsi_list(list, imsis, err);
  REQUIRE(ret == false);
  REQUIRE(imsis.size() == 0);
}


