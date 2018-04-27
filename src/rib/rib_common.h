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

/*! \file    rib_common.h
 *  \brief   utilitiy helper for RIB
 *  \authors Xenofon Foukas
 *  \company Eurecom
 *  \email   x.foukas@sms.ed.ac.uk
 */

#ifndef RIB_COMMON_H_
#define RIB_COMMON_H_

#include <cstdint>
#include <utility>

namespace flexran {

  namespace rib {

    typedef uint16_t frame_t;
    typedef uint16_t subframe_t;

    typedef uint32_t rnti_t;
    
    static int const MAX_NUM_HARQ = 8;
    static int const MAX_NUM_TB = 2;
    static int const MAX_NUM_CC = 2;
    static int const MAX_NUM_LC = 11;
    static int const MAX_NUM_UE = 1024;
    
    static int const N_RBG_MAX = 25; // for 20MHz channel BW
    
#define TBStable_rowCnt 27

#define MAX_SUPPORTED_BW 16

#define CQI_VALUE_RANGE 16
    
    extern const int cqi_to_mcs[16]; 

    extern const unsigned int TBStable[TBStable_rowCnt][110];

    extern const uint8_t cqi2fmt0_agg[MAX_SUPPORTED_BW][CQI_VALUE_RANGE];

    extern const uint8_t cqi2fmt1x_agg[MAX_SUPPORTED_BW][CQI_VALUE_RANGE];

    extern const uint8_t cqi2fmt2x_agg[MAX_SUPPORTED_BW][CQI_VALUE_RANGE];
    
    frame_t get_frame(uint32_t sfn_sf);
    
    subframe_t get_subframe(uint32_t sfn_sf);
    
    uint16_t get_sfn_sf(frame_t frame, subframe_t subframe);
    
    std::pair<frame_t, subframe_t> get_frame_subframe(uint32_t sfn_sf);

  }
  
}
#endif
