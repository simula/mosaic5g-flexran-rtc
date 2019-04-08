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

The optional nodejs and apidoc is not installed which is used to create the
apidoc documentation (see also the [FlexRAN apidoc
documentation](http://mosaic-5g.io/apidocs/flexran/)). To install this, run the
following depending on your distribution:
```bash
$ sudo apt install npm nodejs-legacy # Ubuntu 16.04
$ sudo apt install npm node-gyp nodejs nodejs-dev libssl1.0-dev # Ubuntu 18.04
$ sudo npm install apidoc -g
```

## How to build

To compile issue the command:
```bash
$ ./build_flexran_rtc.sh
```

The controller is configured by default to provide support for real-time
applications as long as a Linux kernel >= 3.14 is available. Disable real-time
support (and run the controller with normal user rights) with the `-r` flag:
```bash
$ ./build_flexran_rtc.sh -r
```

The controller provides a REST northbound API for controlling the deployed
applications using Pistache. Support for this API is enabled by default. If you
want to disable this feature, build with the `-n` flag:
```bash
$ ./build_flexran_rtc.sh -n
```

## How to run

```bash
$ sudo -E ./build/rt_controller -c log_config/basic_log
```
Superuser permissions are required, since changing the scheduling policy from
normal to real-time policies requires execution as root.

To see other options (port for northbound, southbound, configuration), type
```bash
$ ./build/rt_controller -h
```
