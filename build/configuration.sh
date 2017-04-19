# This script is expected to be sourced. It expects the following variables to
# be already set:
#   $PROJECTDIR -- set to the base dir of the project

BSP_NAME="beagleboneblack"
RTEMS_CPU="arm"
RTEMS_VERSION="4.12"
TARGET="${RTEMS_CPU}-rtems${RTEMS_VERSION}"
PREFIX="${PROJECTDIR}/rtems-install/rtems-${RTEMS_VERSION}/"

RTEMS_SOURCE_DIR="${PROJECTDIR}/rtems"
LIBBSD_SOURCE_DIR="${PROJECTDIR}/rtems-libbsd"

BSP_CONFIG_OPT=( \
	"--disable-tests" \
	"--disable-networking")
