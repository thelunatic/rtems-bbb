#! /usr/bin/env sh

# be more verbose
set -x
# exit on wrong command and undefined variables
set -e -u

# find out own directory
SCRIPTDIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROJECTDIR="${SCRIPTDIR}/../"

# configuration
. "${SCRIPTDIR}/configuration.sh"
LOGDIR="${PROJECTDIR}/build/"
NOW="$(date +%Y%m%d_%H%M%S)"
export PATH="${PREFIX}/bin:${PATH}"

export PREFIX

# Build
cd "${DEVICETREE_DIR}"
export DTC_FLAGS="-@"
make "${DTB_FILE}"
mkdir -p "${PREFIX}/fdt/"
cp "${DTB_FILE}" "${PREFIX}/fdt/"

# Create overlays
cd "${DEVICETREEOVERLAY_DIR}"
for f in *.dts
do
	target="${PREFIX}/fdt/`sed 's/\.dts$/.dtbo/' <<<${f}`"
	dtc -@ -o "${target}" ${f}
done
