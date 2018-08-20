/* The MIT License (MIT)

   Copyright (c) 2018 Robert Schmidt

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
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
