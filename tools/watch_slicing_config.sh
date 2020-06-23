#!/bin/bash

host=localhost
port=9999
interval=1

watch -ctn$interval "curl -sX GET http://$host:$port/stats | jq -Cf slicing.jq"
