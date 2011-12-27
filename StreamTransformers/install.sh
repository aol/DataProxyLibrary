#!/bin/bash

set -e

if [ $# -ne 2 ]; then
	echo "usage: $0 <version> <install-dir>"
	exit 1;
fi

version=$1
dir=$2

for f in `find ./ -type f -wholename \*/opt.obj/lib\*`; do
	echo $f | sed "s^\(.*\)/\([^/]*\).so^cp \1/\2.so $dir/\2.so.$version \&\& ln -sfn \2.so.$version $dir/\2.so^" | bash
done
