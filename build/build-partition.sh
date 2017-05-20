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

# Build tools using source builder
cd "${PROJECTDIR}/partition"
make partition
mkdir -p ${PREFIX}/bin/
mv partition ${PREFIX}/bin/
