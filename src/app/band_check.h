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

/*! \file    band_check.h
 *  \brief   Check that band, freq, bandwidth are E-UTRA standard compliant
 *  \authors Robert Schmidt
 *  \company Eurecom
 *  \email   robert.schmidt@eurecom.fr
 */

#ifndef BAND_CHECK_H_
#define BAND_CHECK_H_

/**
 * check for valid E-UTRA bandwidth in RB
 */
bool check_eutra_bandwidth(uint8_t bw_rb, std::string& error);

/**
 * check that frequencies and optionally the band are allowed in this E-UTRA
 * band
 */
bool check_eutra_band(uint8_t band, uint64_t ul_freq, uint64_t dl_freq, std::string& error, uint8_t bw_rb, bool check_bw);

#endif /* BAND_CHECK_H_ */
