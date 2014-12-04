#!/bin/bash

path=.

if [ $# -gt 0 ]; then
	path=$1
fi

watch -n 0.1 -d snmpwalk -v2c -cpublic -m +SPIKE-MIB -M +. localhost:10161 $path
