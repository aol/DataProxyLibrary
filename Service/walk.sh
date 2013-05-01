#!/bin/bash

path=.

if [ $# -gt 0 ]; then
	path=$1
fi

watch -n 0.1 -d snmpwalk -v2c -cpublic -m +SPIKE-MIB -M +cfg/Monitoring localhost:10161 $path
