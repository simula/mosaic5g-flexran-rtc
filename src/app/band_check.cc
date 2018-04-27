/*
 * Copyright 2016-2018 FlexRAN Authors, Eurecom and The University of Edinburgh
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License.  You may obtain a copy
 * of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * For more information about Mosaic5G:  contact@mosaic-5g.io
 */

/*! \file    band_check.cc
 *  \brief   Check that band, freq, bandwidth are E-UTRA standard compliant
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#include <cstdint>
#include <string>
#include "config_common.pb.h"
#include "band_check.h"

struct eutra_band {
  uint8_t  no;
  uint64_t f_ul_low;
  uint64_t f_ul_high;
  uint64_t f_dl_low;
  uint64_t f_dl_high;
  int64_t  f_offset;
  protocol::flex_duplex_mode mode;

  bool     bw_allowed[6]; // corresponds to 1.4, 3, 5, 10, 15, 20 MHz
};

using mode = protocol::flex_duplex_mode;

// see TS 36.101 (version 14.5) tables 5.5-1 and 5.6.1-1
const struct eutra_band eutra_operating_bands[] = {
  {  1, 1920ll * 1000000, 1980ll * 1000000, 2110ll * 1000000, 2170ll * 1000000,  190000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true,  true } },
  {  2, 1850ll * 1000000, 1910ll * 1000000, 1930ll * 1000000, 1990ll * 1000000,   80000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true,  true } },
  {  3, 1710ll * 1000000, 1785ll * 1000000, 1805ll * 1000000, 1880ll * 1000000,   95000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true,  true } },
  {  4, 1710ll * 1000000, 1755ll * 1000000, 2110ll * 1000000, 2155ll * 1000000,  400000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true,  true } },
  {  5,  824ll * 1000000,  849ll * 1000000,  869ll * 1000000,  894ll * 1000000,   45000000ll, mode::FLDM_FDD, {  true,  true,  true,  true, false, false } },
  {  6,  830ll * 1000000,  840ll * 1000000,  875ll * 1000000,  885ll * 1000000,   45000000ll, mode::FLDM_FDD, { false, false,  true,  true, false, false } },
  {  7, 2500ll * 1000000, 2570ll * 1000000, 2620ll * 1000000, 2690ll * 1000000,  120000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true,  true } },
  {  8,  880ll * 1000000,  915ll * 1000000,  925ll * 1000000,  960ll * 1000000,   45000000ll, mode::FLDM_FDD, {  true,  true,  true,  true, false,       } },
  {  9, 17499ll * 100000, 17849ll * 100000, 18449ll * 100000, 18799ll * 100000,   95000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true,  true } },
  { 11, 1710ll * 1000000, 1770ll * 1000000, 2110ll * 1000000, 2170ll * 1000000,  400000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true,  true } },
  { 11, 14279ll * 100000, 14479ll * 100000, 14759ll * 100000, 14959ll * 100000,   48000000ll, mode::FLDM_FDD, { false, false,  true,  true, false, false } },
  { 12,  699ll * 1000000,  716ll * 1000000,  729ll * 1000000,  746ll * 1000000,   30000000ll, mode::FLDM_FDD, {  true,  true,  true,  true, false, false } },
  { 13,  777ll * 1000000,  787ll * 1000000,  746ll * 1000000,  756ll * 1000000,  -31000000ll, mode::FLDM_FDD, { false, false,  true,  true, false, false } },
  { 14,  788ll * 1000000,  798ll * 1000000,  758ll * 1000000,  768ll * 1000000,  -30000000ll, mode::FLDM_FDD, { false, false,  true,  true, false, false } },
  //15  Reserved
  //16  Reserved
  { 17,  704ll * 1000000,  716ll * 1000000,  734ll * 1000000,  746ll * 1000000,   30000000ll, mode::FLDM_FDD, { false, false,  true,  true, false, false } },
  { 18,  815ll * 1000000,  830ll * 1000000,  860ll * 1000000,  875ll * 1000000,   45000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true, false } },
  { 19,  830ll * 1000000,  845ll * 1000000,  875ll * 1000000,  890ll * 1000000,   45000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true, false } },
  { 20,  832ll * 1000000,  862ll * 1000000,  791ll * 1000000,  821ll * 1000000,  -41000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true,  true } },
  { 21, 14479ll * 100000, 14629ll * 100000, 14959ll * 100000, 15109ll * 100000,   48000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true, false } },
  { 22, 3410ll * 1000000, 3490ll * 1000000, 3510ll * 1000000, 3590ll * 1000000,  100000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true,  true } },
  { 23, 2000ll * 1000000, 2020ll * 1000000, 2180ll * 1000000, 2200ll * 1000000,  180000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true,  true } },
  { 24, 16265ll * 100000, 16605ll * 100000, 1525ll * 1000000, 1559ll * 1000000, -101500000ll, mode::FLDM_FDD, { false, false,  true,  true, false, false } },
  { 25, 1850ll * 1000000, 1915ll * 1000000, 1930ll * 1000000, 1995ll * 1000000,   80000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true,  true } },
  { 26,  814ll * 1000000,  849ll * 1000000,  859ll * 1000000,  894ll * 1000000,   45000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true, false } },
  { 27,  807ll * 1000000,  824ll * 1000000,  852ll * 1000000,  869ll * 1000000,   45000000ll, mode::FLDM_FDD, {  true,  true,  true,  true, false, false } },
  { 28,  703ll * 1000000,  748ll * 1000000,  758ll * 1000000,  803ll * 1000000,   55000000ll, mode::FLDM_FDD, { false,  true,  true,  true,  true,  true } },
  //29  Reserved for DL CA
  { 30, 2305ll * 1000000, 2315ll * 1000000, 2350ll * 1000000, 2360ll * 1000000,   45000000ll, mode::FLDM_FDD, { false, false,  true,  true, false, false } },
  { 31,  4525ll * 100000,  4575ll * 100000,  4625ll * 100000,  4675ll * 100000,   10000000ll, mode::FLDM_FDD, {  true,  true,  true, false, false, false } },
  //32  Reserved for DL CA
  { 33, 1900ll * 1000000, 1920ll * 1000000, 1900ll * 1000000, 1920ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 34, 2010ll * 1000000, 2025ll * 1000000, 2010ll * 1000000, 2025ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true, false } },
  { 35, 1850ll * 1000000, 1910ll * 1000000, 1850ll * 1000000, 1910ll * 1000000,          0ll, mode::FLDM_TDD, {  true,  true,  true,  true,  true,  true } },
  { 36, 1930ll * 1000000, 1990ll * 1000000, 1930ll * 1000000, 1990ll * 1000000,          0ll, mode::FLDM_TDD, {  true,  true,  true,  true,  true,  true } },
  { 37, 1910ll * 1000000, 1930ll * 1000000, 1910ll * 1000000, 1930ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 38, 2570ll * 1000000, 2620ll * 1000000, 2570ll * 1000000, 2620ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 39, 1880ll * 1000000, 1920ll * 1000000, 1880ll * 1000000, 1920ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 40, 2300ll * 1000000, 2400ll * 1000000, 2300ll * 1000000, 2400ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 41, 2496ll * 1000000, 2690ll * 1000000, 2496ll * 1000000, 2690ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 42, 3400ll * 1000000, 3600ll * 1000000, 3400ll * 1000000, 3600ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 43, 3600ll * 1000000, 3800ll * 1000000, 3600ll * 1000000, 3800ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 44,  703ll * 1000000,  803ll * 1000000,  703ll * 1000000,  803ll * 1000000,          0ll, mode::FLDM_TDD, { false,  true,  true,  true,  true,  true } },
  { 45, 1447ll * 1000000, 1467ll * 1000000, 1447ll * 1000000, 1467ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  { 46, 5150ll * 1000000, 5925ll * 1000000, 5150ll * 1000000, 5925ll * 1000000,          0ll, mode::FLDM_TDD, { false, false, false,  true, false,  true } },
  { 47, 5855ll * 1000000, 5925ll * 1000000, 5855ll * 1000000, 5925ll * 1000000,          0ll, mode::FLDM_TDD, { false, false, false,  true, false,  true } },
  { 48, 3550ll * 1000000, 3700ll * 1000000, 3550ll * 1000000, 3700ll * 1000000,          0ll, mode::FLDM_TDD, { false, false,  true,  true,  true,  true } },
  //64  Reserved
  { 65, 1920ll * 1000000, 2010ll * 1000000, 2110ll * 1000000, 2200ll * 1000000,  190000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true,  true } },
  { 66, 1710ll * 1000000, 1780ll * 1000000, 2110ll * 1000000, 2200ll * 1000000,  400000000ll, mode::FLDM_FDD, {  true,  true,  true,  true,  true,  true } }, // DL 2180 - 2200 MHz reserved for DL CA
  //67  Reserved for DL CA
  { 68,  698ll * 1000000,  728ll * 1000000,  753ll * 1000000,  783ll * 1000000,   55000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true, false } },
  //69  Reserved for DL CA
  { 70, 1695ll * 1000000, 1710ll * 1000000, 1995ll * 1000000, 2020ll * 1000000,  300000000ll, mode::FLDM_FDD, { false, false,  true,  true,  true,  true } },
};

int get_eutra_bw_index(uint8_t bw_rb)
{
  switch (bw_rb) {
    case   6: return 0; //  1.4 MHz
    case  15: return 1; //  3   MHz
    case  25: return 2; //  5   MHz
    case  50: return 3; // 10   MHz
    case  75: return 4; // 15   MHz
    case 100: return 5; // 20   MHz
    default:  return -1;
  }
}

int get_eutra_band_index(uint8_t band)
{
  for (size_t i = 0; i < sizeof(eutra_operating_bands) / sizeof(struct eutra_band); ++i) {
    if (band == eutra_operating_bands[i].no)
      return i;
  }
  return -1;
}

bool check_eutra_bandwidth(uint8_t bw_rb, std::string& error)
{
  if (get_eutra_bw_index(bw_rb) < 0) {
    error = "illegal E-UTRA bandwidth " + std::to_string(bw_rb) + ", must be in (6, 15, 25, 50, 75, 100)";
    return false;
  }
  return true;
}

bool check_eutra_band(uint8_t band, uint64_t ul_freq, uint64_t dl_freq, std::string& error, uint8_t bw_rb = 0, bool check_bw = false)
{
  int idx = get_eutra_band_index(band);
  if (idx < 0) {
    error = "illegal E-UTRA band " + std::to_string(band);
    return false;
  }

  /* verify UL range */
  if (ul_freq < eutra_operating_bands[idx].f_ul_low || ul_freq >= eutra_operating_bands[idx].f_ul_high) {
    error = "illegal ul_freq " + std::to_string(ul_freq) + ", must be within ["
        + std::to_string(eutra_operating_bands[idx].f_ul_low) + ","
        + std::to_string(eutra_operating_bands[idx].f_ul_high) + ") for E-UTRA band " + std::to_string(band);
    return false;
  }

  /* verify DL range */
  if (dl_freq < eutra_operating_bands[idx].f_dl_low || dl_freq >= eutra_operating_bands[idx].f_dl_high) {
    error = "illegal dl_freq " + std::to_string(dl_freq) + ", must be within ["
        + std::to_string(eutra_operating_bands[idx].f_dl_low) + ","
        + std::to_string(eutra_operating_bands[idx].f_dl_high) + ") for E-UTRA band " + std::to_string(band);
    return false;
  }

  /* verify offest between DL/UL */
  if ((eutra_operating_bands[idx].f_offset < 0 && ul_freq - dl_freq != (uint64_t)-eutra_operating_bands[idx].f_offset)
      || (dl_freq - ul_freq != (uint64_t)eutra_operating_bands[idx].f_offset)) {
    error = "offset of ul_freq and dl_freq must "
        + std::to_string(eutra_operating_bands[idx].f_offset)
        + " for E-UTRA band " + std::to_string(eutra_operating_bands[idx].no);
    return false;
  }

  if (check_bw) {
    int idx_bw = get_eutra_bw_index(bw_rb);
    if (idx_bw < 0) {
      error = "illegal E-UTRA bandwidth " + std::to_string(bw_rb) + ", must be in (6, 15, 25, 50, 75, 100)";
      return false;
    }
    if (!eutra_operating_bands[idx].bw_allowed[idx_bw]) {
      error = "E-UTRA bandwidth of " + std::to_string(bw_rb) + " not allowed for E-UTRA band " + std::to_string(band);
      return false;
    }
  }

  return true;
}
