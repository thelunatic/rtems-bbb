#!/bin/sh

set -e
set -u
set -x

revision=6b0307a0a5184339393f555d5d424190d8a8277a

if [ ! -e ${revision}.tar.gz ]
then
	curl -LO https://github.com/freebsd/freebsd/archive/${revision}.tar.gz
fi
tar xf ${revision}.tar.gz

if [ -e sys ]
then
	rm -r sys
fi
mkdir -p sys/gnu
mkdir -p sys/conf
mkdir -p sys/tools
cp -r freebsd-${revision}/sys/gnu/dts sys/gnu
cp -r freebsd-${revision}/sys/dts sys
cp -r freebsd-${revision}/sys/tools/fdt sys/tools
rm -r freebsd-${revision}
