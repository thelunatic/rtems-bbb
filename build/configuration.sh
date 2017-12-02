# This script is expected to be sourced. It expects the following variables to
# be already set:
#   $PROJECTDIR -- set to the base dir of the project

BSP_NAME="beagleboneblack"
RTEMS_CPU="arm"
RTEMS_VERSION="5"
TARGET="${RTEMS_CPU}-rtems${RTEMS_VERSION}"
PREFIX="${PROJECTDIR}/install/rtems/${RTEMS_VERSION}/"
UBOOT_CONFIG="am335x_evm"
DTB_FILE="src/arm/am335x-boneblack.dtb"
DTB_INSTALL_NAME="am335x-boneblack.dtb"

RSB_DIR="${PROJECTDIR}/tools/rtems-source-builder"
RTEMS_SOURCE_DIR="${PROJECTDIR}/libs/rtems"
LIBBSD_SOURCE_DIR="${PROJECTDIR}/libs/rtems-libbsd"
U_BOOT_SOURCE_DIR="${PROJECTDIR}/tools/u-boot"
NEWFS_MSDOS_SOURCE_DIR="${PROJECTDIR}/tools/newfs_msdos"
DEVICETREE_DIR="${PROJECTDIR}/tools/device-tree-rebasing"
MTOOLS_DIR="${PROJECTDIR}/tools/mtools"
PARTITION_DIR="${PROJECTDIR}/tools/partition"

BSP_CONFIG_OPT="
	--enable-tests=samples
	--disable-networking
	CONSOLE_POLLED=1
	"
