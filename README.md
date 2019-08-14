# FlexRAN-rtc

A real-time controller for SD-RAN

## Build requirements

Requirements (in parenthesis: the exact package names on Ubuntu)
* Cmake >= 3.5
* libprotobuf v3.3.0
* boost >= 1.54 (libboost-system-dev, libboost-program-options-dev)
* log4cxx >= 0.10.0 (liblog4cxx-dev liblog4cxx10v5)
* curl-dev (libcurl4-openssl-dev)
* Compiler with support for C++11
* Pistache library (e.g. later than Nov 22, 2017) for RESTful northbound API support
* Optionally: nodejs >= 4.2.6

Basic dependencies can be automatically installed by running the bash script
`install_dependencies` located in the tools directory. The current version of
the script only provides support for Ubuntu 16.04 and 18.04 (other Ubuntu
versions might or might not work). For other Linux distributions, please install
these packages manually using your package manager.

The optional nodejs and apidoc used to create the apidoc documentation (see
also the [FlexRAN apidoc documentation](http://mosaic-5g.io/apidocs/flexran/))
is not automatically installed. To install and run apidoc, use:
```bash
$ sudo apt install npm nodejs-legacy # Ubuntu 16.04
$ sudo apt install npm node-gyp nodejs nodejs-dev libssl1.0-dev # Ubuntu 18.04
$ sudo npm install apidoc -g
$ apidoc -i src/ -o ./docs -f ".*\\.cc$" -f ".*\\.h$"
```

## How to build

To compile, issue the command:
```bash
$ ./build_flexran_rtc.sh
```

It is a shorthand for the standard CMake compile usage:
```bash
$ mkdir build
$ cd build
$ cmake ..
$ make -j$(nproc)
```

A number of options are available during build configuration (default is in
brackets):

*  `LOWLATENCY=[ON]|OFF`: toggle real-time support, depends on Linux >= 3.14.
*  `REST_NORTHBOUND=[ON]|OFF`: toggle the northbound interface on/off.
*  `PROFILE=ON|[OFF]`: toggle profiling. During runtime, send USR1 signal to
   the RTC to trigger measurement. It will report number of packets/bytes
   received from its agent(s) and the loop duration of applications.
*  (legacy) `NEO4J_SUPPORT=ON|[OFF]`: toggle Neo4J integration, likely not
   functional anymore.
*  `ENABLE_TESTS=ON|[OFF]`: enable or disable tests. Run with `check` command
   in the build directory.

To use one of these options, pass it to CMake like so:
```bash
$ cmake -DLOWLATENCY=OFF ..
```

## How to run

To run, issue one of the commands:
```bash
$ ./run_flexran_rtc.sh
$ sudo ./build/rt_controller -c log_config/basic_log
$ ./build/rt_controller -h   # retrieve help
```
