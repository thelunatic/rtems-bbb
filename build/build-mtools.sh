#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# configuration
source "${SCRIPTDIR}/configuration.sh"
LOGDIR="${PROJECTDIR}/build/"
NOW="$(date +%Y%m%d_%H%M%S)"
export PATH="${PREFIX}/bin:${PATH}"

export PREFIX

# Build mtools
cd ${PROJECTDIR}/mtools
[[ -e build ]] && rm -rf build
mkdir build
cd build
tar xaf ${PROJECTDIR}/mtools/mtools-4.0.18.tar.bz2
cd mtools-4.0.18
patch -p1 -i ${PROJECTDIR}/mtools/mtools-4.0.18.diff

./configure --prefix=${PREFIX}
make -j `nproc` all
make install
