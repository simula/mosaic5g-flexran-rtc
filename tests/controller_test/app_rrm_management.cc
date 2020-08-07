#include "catch.hpp"
#include "rrm_management.h"
#include <vector>
#include <iostream>
#include <regex>

using rrm = flexran::app::management::rrm_management;

TEST_CASE("test correct parsing of IMSIs", "[rrm_management]")
{
  std::vector<std::regex> imsi;
  bool ret;
  std::string err;

  // regex tests

  std::string list = " \t[\"2089500[0-9]*[0-6]\"]  ";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 1);
  ret=std::regex_search ("208950052501016",imsi.at(0));
  REQUIRE(ret == true);

  imsi.clear();
  list ="\t[\"^20894[0-9 m]+101$\"]";
  ret = rrm::parse_imsi_reg(list, imsi , err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);
  

  list = "[\"^208950000000002$\"]  ";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 1);
  ret=std::regex_search ("208950000000002",imsi.at(0));
  REQUIRE(ret == true);

  imsi.clear();
  list = "\t[\"[0-9]*10101[  0-9]*\"]\t";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 1);
  ret=std::regex_search ("20895010100100002",imsi.at(0));
  REQUIRE(ret == false);
  REQUIRE(std::regex_search ("2089501010100002", imsi.at(0)));

  imsi.clear();
  list = "   [\"^20894[0-9+  101$\"]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);

  list = "[\"^20894[0-9]a+101a$\"]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);	

  list = "[\"2089500[0-9]*[0-6]\",\"^208950000000002$\",\"[0 -9]*10101[0-9]*\",\"^20894[0-9]+101$\",\"208950  011\"]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 5);
  ret=std::regex_search  ("2089511010101",imsi.at(2));
  REQUIRE(ret == true);
 
  imsi.clear();
  list = "[   \"2089500[0-9]*[0-6]\",   208950000000002, ,,]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 2);
  REQUIRE(std::regex_search("208950000000002", imsi.at(0)));
  REQUIRE(std::regex_search("208950000000002", imsi.at(1)));

  imsi.clear();
  list = "[   ,   208950000000002,[ ,,]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);


  // simple tests
  imsi.clear();
  list = "";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);

  list = "123";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);

  list = "  [   ]   ";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 0);
  
  imsi.clear();
  list = "[124]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 1);
  REQUIRE(!std::regex_search ("1", imsi.at(0)));
  REQUIRE(std::regex_search ("124", imsi.at(0)));
  REQUIRE(!std::regex_search ("123", imsi.at(0)));
  

  imsi.clear();
  list = "[\n\n\n    \t123, \n      123451]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == true);
  REQUIRE(imsi.size() == 2);
  REQUIRE(std::regex_search("1234", imsi.at(0)));
 

  imsi.clear();
  list = "[\n\n\n    \t123, \n      123451a]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);

  list = "[\n\n\n    \t123, a\n      123451]";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);

  list = "[\n\n\n    \t123, \n      123451,";
  ret = rrm::parse_imsi_reg(list, imsi, err);
  REQUIRE(ret == false);
  REQUIRE(imsi.size() == 0);
}
