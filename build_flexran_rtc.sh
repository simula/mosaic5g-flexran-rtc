#!/bin/bash

source ./flexran_rtc_env

COMMAND="cmake"
ARGS=""

unset $NO_REST
unset $NO_REALTIME
unset $YES_NEO4J

rm "$FLEXRAN_RTC_HOME/CMakeCache.txt"

while getopts rnj option
do
    case "${option}"
    in
        r) NO_REALTIME=1;;
        n) NO_REST=1;;
        j) YES_NEO4J=1;;
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

if [ -z $YES_NEO4J ]; then
    echo "Compiling without Neo4J support"
else
    echo "Compiling with Neo4J support"
    ARGS="$ARGS -DNEO4J_SUPPORT=ON"
fi

cmake $ARGS . && make
