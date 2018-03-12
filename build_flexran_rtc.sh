#!/bin/bash

source ./flexran_rtc_env

COMMAND="cmake"
ARGS=""

unset $NO_REST
unset $NO_REALTIME
unset $YES_NEO4J
unset $NO_APIDOCS

rm -f "$FLEXRAN_RTC_HOME/CMakeCache.txt"

function print_help() {
  echo '
This program builds FlexRAN RTC based on the following options.
Options
-h
   print this help
-r
  disable realtime tasks
-n
  diable the northbound REST interface
-j
  enable interface to neo4j (deprecated)
-d
 disable auogeneration of API docs
Usage:
- build_flexran_rtc
'
  exit 0
}

while getopts rnjdh option
do
    case "${option}"
    in
        r) NO_REALTIME=1;;
        n) NO_REST=1;;
        j) YES_NEO4J=1;;
	d) NO_APIDOCS=1;;
	h) print_help;;
    esac
done

if [ -z $NO_REALTIME ]; then
    echo "Compiling with real-time support"
else
    echo "Compiling without real-time support"
    ARGS="$ARGS -DLOWLATENCY=OFF"
fi

if [ -z $NO_REST ]; then
    echo "Compiling with REST API enabled"
else
    echo "Compiling without REST API"
    ARGS="$ARGS -DREST_NORTHBOUND=OFF"
fi

if [ -z $NO_APIDOCS ]; then
    echo "Generating API documentation"
    apidoc -i src/ -o ./docs -f ".*\\.cc$" -f ".*\\.h$"
else
    echo "Do not generate API documentation"
fi

if [ -z $YES_NEO4J ]; then
    echo "Compiling without Neo4J support"
else
    echo "Compiling with Neo4J support"
    ARGS="$ARGS -DNEO4J_SUPPORT=ON"
fi

cmake $ARGS . && make
