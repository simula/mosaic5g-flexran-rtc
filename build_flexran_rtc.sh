#!/bin/bash

source ./flexran_rtc_env

rm -r CTestTestfile.cmake CMakeFiles/ CMakeCache.txt build cmake_install.cmake
mkdir build && cd build && cmake .. && make -j "$(nproc)"
