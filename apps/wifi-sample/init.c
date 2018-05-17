/*
 * Copyright (c) 2017 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdlib.h>

#include <rtems.h>
#include <bsp.h>

#include "libbsdhelper.h"

#define PRIO_SHELL		150
#define PRIO_WPA		(RTEMS_MAXIMUM_PRIORITY - 1)
#define PRIO_INIT_TASK		(RTEMS_MAXIMUM_PRIORITY - 1)
#define PRIO_MEDIA_SERVER	200

const char *wpa_supplicant_conf = "/media/mmcsd-0-0/wpa_supplicant.conf";
char *wlandev = "rtwn0";

static void
Init(rtems_task_argument arg)
{
	rtems_status_code sc;

	(void)arg;

	puts("\nRTEMS WiFi Demo\n");
	libbsdhelper_init_sd_card(PRIO_MEDIA_SERVER);
	libbsdhelper_lower_self_prio(PRIO_INIT_TASK);
	libbsdhelper_init_libbsd();

	/* Wait for the SD card */
	sc = libbsdhelper_wait_for_sd();
	if (sc == RTEMS_SUCCESSFUL) {
		printf("SD: OK\n");
	} else {
		printf("ERROR: SD could not be mounted after timeout\n");
	}

	rtems_dhcpcd_start(NULL);
	printf("Mode 1 set: Create WLAN device.\n");
	/* Some time for USB device to be detected. */
	rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(4000));
	libbsdhelper_create_wlandev(wlandev);

	libbsdhelper_init_wpa_supplicant(wpa_supplicant_conf, PRIO_WPA);

	libbsdhelper_start_shell(PRIO_SHELL);

	exit(0);
}

/*
 * Configure LibBSD.
 */
#define RTEMS_BSD_CONFIG_BSP_CONFIG
#define RTEMS_BSD_CONFIG_TERMIOS_KQUEUE_AND_POLL
#define RTEMS_BSD_CONFIG_INIT

#include <machine/rtems-bsd-config.h>

/*
 * Configure RTEMS.
 */
#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_STUB_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_ZERO_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_LIBBLOCK

#define CONFIGURE_FILESYSTEM_DOSFS
#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 32

#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 1

#define CONFIGURE_INIT_TASK_STACK_SIZE (64*1024)
#define CONFIGURE_INIT_TASK_INITIAL_MODES RTEMS_DEFAULT_MODES
#define CONFIGURE_INIT_TASK_ATTRIBUTES RTEMS_FLOATING_POINT

#define CONFIGURE_BDBUF_BUFFER_MAX_SIZE (32 * 1024)
#define CONFIGURE_BDBUF_MAX_READ_AHEAD_BLOCKS 4
#define CONFIGURE_BDBUF_CACHE_MEMORY_SIZE (1 * 1024 * 1024)
#define CONFIGURE_BDBUF_READ_AHEAD_TASK_PRIORITY 97
#define CONFIGURE_SWAPOUT_TASK_PRIORITY 97

//#define CONFIGURE_STACK_CHECKER_ENABLED

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT

#include <rtems/confdefs.h>

/*
 * Configure Shell.
 */
#include <rtems/netcmds-config.h>
#include <bsp/irq-info.h>
#define CONFIGURE_SHELL_COMMANDS_INIT

#define CONFIGURE_SHELL_USER_COMMANDS \
  &bsp_interrupt_shell_command, \
  &rtems_shell_ARP_Command, \
  &rtems_shell_PFCTL_Command, \
  &rtems_shell_PING_Command, \
  &rtems_shell_IFCONFIG_Command, \
  &rtems_shell_ROUTE_Command, \
  &rtems_shell_NETSTAT_Command, \
  &rtems_shell_DHCPCD_Command, \
  &rtems_shell_HOSTNAME_Command, \
  &rtems_shell_SYSCTL_Command, \
  &rtems_shell_VMSTAT_Command, \
  &rtems_shell_WLANSTATS_Command, \
  &rtems_shell_STARTFTP_Command, \
  &rtems_shell_BLKSTATS_Command, \
  &rtems_shell_WPA_SUPPLICANT_Command, \
  &rtems_shell_WPA_SUPPLICANT_FORK_Command

#define CONFIGURE_SHELL_COMMANDS_ALL

#include <rtems/shellconfig.h>
