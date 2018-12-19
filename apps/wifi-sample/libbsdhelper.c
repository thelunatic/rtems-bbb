/*
 * Copyright (c) 2016 embedded brains GmbH.  All rights reserved.
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

#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sysexits.h>

#include <machine/rtems-bsd-commands.h>

#include <rtems.h>
#include <rtems/bsd/bsd.h>
#include <rtems/bdbuf.h>
#include <rtems/console.h>
#include <rtems/irq.h>
#include <rtems/malloc.h>
#include <rtems/media.h>
#include <rtems/score/armv7m.h>
#include <rtems/stringto.h>
#include <rtems/shell.h>
#include <rtems/ftpd.h>

#include <bsp.h>
#include <bsp/irq.h>

#include "libbsdhelper.h"

#define STACK_SIZE_MEDIA_SERVER	(64 * 1024)
#define STACK_SIZE_IRQ_SERVER	(8 * 1024)
#define STACK_SIZE_SHELL	(64 * 1024)

#define EVT_MOUNTED		RTEMS_EVENT_9

static rtems_id wait_mounted_task_id = RTEMS_INVALID_ID;

static rtems_status_code
media_listener(rtems_media_event event, rtems_media_state state,
    const char *src, const char *dest, void *arg)
{
	printf(
		"media listener: event = %s, state = %s, src = %s",
		rtems_media_event_description(event),
		rtems_media_state_description(state),
		src
	);

	if (dest != NULL) {
		printf(", dest = %s", dest);
	}

	if (arg != NULL) {
		printf(", arg = %p\n", arg);
	}

	printf("\n");

	if (event == RTEMS_MEDIA_EVENT_MOUNT &&
	    state == RTEMS_MEDIA_STATE_SUCCESS) {
		rtems_event_send(wait_mounted_task_id, EVT_MOUNTED);
	}

	return RTEMS_SUCCESSFUL;
}

rtems_status_code
libbsdhelper_wait_for_sd(void)
{
	puts("waiting for SD...\n");
	rtems_status_code sc;
	const rtems_interval max_mount_time = 3000 /
	    rtems_configuration_get_milliseconds_per_tick();
	rtems_event_set out;

	sc = rtems_event_receive(EVT_MOUNTED, RTEMS_WAIT, max_mount_time, &out);
	assert(sc == RTEMS_SUCCESSFUL || sc == RTEMS_TIMEOUT);

	return sc;
}

void
libbsdhelper_lower_self_prio(rtems_task_priority new)
{
	rtems_status_code sc;
	rtems_task_priority oldprio;

	/* Let other tasks run to complete background work */
	sc = rtems_task_set_priority(RTEMS_SELF,
	    new, &oldprio);
	assert(sc == RTEMS_SUCCESSFUL);
}

void
libbsdhelper_init_sd_card(rtems_task_priority prio_mediaserver)
{
	rtems_status_code sc;

	wait_mounted_task_id = rtems_task_self();

	sc = rtems_bdbuf_init();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_listener_add(media_listener, NULL);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_media_server_initialize(
	    prio_mediaserver,
	    STACK_SIZE_MEDIA_SERVER,
	    RTEMS_DEFAULT_MODES,
	    RTEMS_DEFAULT_ATTRIBUTES
	    );
	assert(sc == RTEMS_SUCCESSFUL);
}

struct rtems_ftpd_configuration rtems_ftpd_configuration = {
	.priority = 100,
	.max_hook_filesize = 0,
	.port = 21,
	.hooks = NULL,
	.root = NULL,
	.tasks_count = 4,
	.idle = 5 * 60,
	.access = 0
};

static int
command_startftp(int argc, char *argv[])
{
	rtems_status_code sc;

	(void) argc;
	(void) argv;

	sc = rtems_initialize_ftpd();
	if(sc == RTEMS_SUCCESSFUL) {
		printf("FTP started.\n");
	} else {
		printf("ERROR: FTP could not be started.\n");
	}

	return 0;
}

rtems_shell_cmd_t rtems_shell_STARTFTP_Command = {
	"startftp",          /* name */
	"startftp",          /* usage */
	"net",               /* topic */
	command_startftp,    /* command */
	NULL,                /* alias */
	NULL,                /* next */
	0, 0, 0
};

void
libbsdhelper_start_shell(rtems_task_priority prio)
{
	rtems_status_code sc = rtems_shell_init(
		"SHLL",
		STACK_SIZE_SHELL,
		prio,
		CONSOLE_DEVICE_NAME,
		false,
		true,
		NULL
	);
	assert(sc == RTEMS_SUCCESSFUL);
}

void
libbsdhelper_create_wlandev(char *dev)
{
	int exit_code;
	char *ifcfg[] = {
		"ifconfig",
		"wlan0",
		"create",
		"wlandev",
		dev,
		"up",
		NULL
	};

	exit_code = rtems_bsd_command_ifconfig(RTEMS_BSD_ARGC(ifcfg), ifcfg);
	if(exit_code != EXIT_SUCCESS) {
		printf("ERROR while creating wlan0.");
	}
}

static char *wpa_supplicant_cmd[] = {
	"wpa_supplicant",
	"-Dbsd",
	"-iwlan0",
	"-c",
	NULL, /* Will be replaced with path to config. */
	NULL
};

static void
wpa_supplicant_watcher_task(rtems_task_argument arg)
{
	int argc;
	char ** argv;
	int err;
	(void) arg;

	argv = wpa_supplicant_cmd;
	argc = sizeof(wpa_supplicant_cmd)/sizeof(wpa_supplicant_cmd[0])-1;

	while (true) {
		rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(2000));
		err = rtems_bsd_command_wpa_supplicant(argc, argv);
		printf("wpa_supplicant returned with %d\n", err);
	}
}

static int file_exists(const char *filename) {
    struct stat st;
    int result = stat(filename, &st);
    return result == 0;
}

/*
 * FIXME: This currently starts an ugly hack (wpa_supplicant_watcher) that just
 * restarts the wpa_supplicant if it doesn't run anymore. It should be replaced
 * by a proper event handling.
 */
void
libbsdhelper_init_wpa_supplicant(const char *confnam, rtems_task_priority prio)
{
	char *conf;
	size_t pos;
	rtems_status_code sc;
	rtems_id id;

	if (!file_exists(confnam)) {
		printf("ERROR: wpa configuration does not exist: %s\n",confnam);
		return;
	}

	pos = sizeof(wpa_supplicant_cmd)/sizeof(wpa_supplicant_cmd[0]) - 2;
	conf = strdup(confnam);
	wpa_supplicant_cmd[pos] = conf;

	sc = rtems_task_create(
		rtems_build_name('W', 'P', 'A', 'W'),
		prio,
		32 * 1024,
		RTEMS_DEFAULT_MODES,
		RTEMS_FLOATING_POINT,
		&id
	);
	assert(sc == RTEMS_SUCCESSFUL);

	sc = rtems_task_start(id, wpa_supplicant_watcher_task, 0);
	assert(sc == RTEMS_SUCCESSFUL);
}
