/*
 * Copyright (c) 2019 Vijay Kumar Banerjee. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *	  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *	  notice, this list of conditions and the following disclaimer in the
 *	  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.	IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include <stdlib.h>
#include <sysexits.h>

#include <rtems.h>
#include <rtems/bsd/bsd.h>
#include <rtems/dhcpcd.h>
#include <bsp/i2c.h>
#include <libcpu/am335x.h>
#include <rtems/irq-extension.h>
#include <rtems/counter.h>
#include <bsp/bbb-gpio.h>
#include <rtems/console.h>
#include <rtems/shell.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dev/iicbus/iic.h>

#include <machine/rtems-bsd-commands.h>

#include <bsp.h>

#define PRIO_SHELL		150
#define PRIO_WPA		(RTEMS_MAXIMUM_PRIORITY - 1)
#define PRIO_INIT_TASK		(RTEMS_MAXIMUM_PRIORITY - 1)
#define PRIO_MEDIA_SERVER	200
#define STACK_SIZE_SHELL	(64 * 1024)
#define I2C_BUS "/dev/iic0"
#define EEPROM_ADDRESS 0x50

#define XORMODE	0x80000000

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>

#include <sys/consio.h>
#include <sys/fbio.h>

int open_framebuffer(void);
int close_framebuffer(void);
void setcolor(unsigned colidx, unsigned value);
void pixel (int x, int y, unsigned colidx);
void line (int x1, int y1, int x2, int y2, unsigned colidx);
void rect (int x1, int y1, int x2, int y2, unsigned colidx);
void fillrect (int x1, int y1, int x2, int y2, unsigned colidx);

union multiptr {
	unsigned char *p8;
	unsigned short *p16;
	unsigned long *p32;
};

static int fbsize;
static unsigned char *fbuffer;
static unsigned char **line_addr;
static int fb_fd=0;
static int bytes_per_pixel;
static unsigned colormap [256];
uint32_t xres, yres;

static char *defaultfbdevice = "/dev/fb0";
static char *fbdevice = NULL;

int open_framebuffer(void)
{
	int y;
	unsigned addr;
	struct fbtype fb;
	int line_length;

	if ((fbdevice = getenv ("TSLIB_FBDEVICE")) == NULL)
		fbdevice = defaultfbdevice;

	fb_fd = open(fbdevice, O_RDWR);
	if (fb_fd == -1) {
		perror("open fbdevice");
		return -1;
	}


	if (ioctl(fb_fd, FBIOGTYPE, &fb) != 0) {
		perror("ioctl(FBIOGTYPE)");
		return -1;
	}

	if (ioctl(fb_fd, FBIO_GETLINEWIDTH, &line_length) != 0) {
		perror("ioctl(FBIO_GETLINEWIDTH)");
		return -1;
	}

	xres = fb.fb_width;
	yres = fb.fb_height;

	int pagemask = getpagesize() - 1;
	fbsize = ((int) line_length*yres + pagemask) & ~pagemask;

	fbuffer = (unsigned char *)mmap(0, fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (fbuffer == (unsigned char *)-1) {
		perror("mmap framebuffer");
		close(fb_fd);
		return -1;
	}
	memset(fbuffer,0, fbsize);

	bytes_per_pixel = (fb.fb_depth + 7) / 8;
	line_addr = malloc (sizeof (uint32_t) * fb.fb_height);
	addr = 0;
	for (y = 0; y < fb.fb_height; y++, addr += line_length)
		line_addr [y] = fbuffer + addr;

	return 0;
}

int close_framebuffer(void)
{
	munmap(fbuffer, fbsize);
	close(fb_fd);
	free (line_addr);

	return (0);
}

void setcolor(unsigned colidx, unsigned value)
{
	unsigned res;
	unsigned char red, green, blue;

#ifdef DEBUG
	if (colidx > 255) {
		fprintf (stderr, "WARNING: color index = %u, must be <256\n",
			 colidx);
		return;
	}
#endif

	red = (value >> 16) & 0xff;
	green = (value >> 8) & 0xff;
	blue = value & 0xff;

	switch (bytes_per_pixel) {
	case 1:
		res = value;
		break;
	case 2:
		res = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
		break;
	case 3:
	case 4:
	default:
		res = value;

	}
		colormap [colidx] = res;
}

static inline void __setpixel (union multiptr loc, unsigned xormode, unsigned color)
{
	switch(bytes_per_pixel) {
	case 1:
	default:
		if (xormode)
			*loc.p8 ^= color;
		else
			*loc.p8 = color;
		pwrite(fb_fd, loc.p8, sizeof(loc.p16), 0);
		break;
	case 2:
		if (xormode)
			*loc.p16 ^= color;
		else
			*loc.p16 = color;
		pwrite(fb_fd, loc.p16, sizeof(loc.p16), 0);
		break;
	case 3:
		if (xormode) {
			*loc.p8++ ^= (color >> 16) & 0xff;
			*loc.p8++ ^= (color >> 8) & 0xff;
			*loc.p8 ^= color & 0xff;
		} else {
			*loc.p8++ = (color >> 16) & 0xff;
			*loc.p8++ = (color >> 8) & 0xff;
			*loc.p8 = color & 0xff;
		}
		pwrite(fb_fd, loc.p8, sizeof(loc.p8), 0);
		break;
	case 4:
		if (xormode)
			*loc.p32 ^= color;
		else
			*loc.p32 = color;
		pwrite(fb_fd, loc.p32, sizeof(loc.p32), 0);
		break;
	}
}

void pixel (int x, int y, unsigned colidx)
{
	unsigned xormode;
	union multiptr loc;

	if ((x < 0) || ((uint32_t)x >= xres) ||
		(y < 0) || ((uint32_t)y >= yres))
		return;

	xormode = colidx & XORMODE;
	colidx &= ~XORMODE;

#ifdef DEBUG
	if (colidx > 255) {
		fprintf (stderr, "WARNING: color value = %u, must be <256\n",
			 colidx);
		return;
	}
#endif

	loc.p8 = line_addr [y] + x * bytes_per_pixel;
	__setpixel (loc, xormode, colormap [colidx]);
}

void line (int x1, int y1, int x2, int y2, unsigned colidx)
{
	int tmp;
	int dx = x2 - x1;
	int dy = y2 - y1;

	if (abs (dx) < abs (dy)) {
		if (y1 > y2) {
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		x1 <<= 16;
		/* dy is apriori >0 */
		dx = (dx << 16) / dy;
		while (y1 <= y2) {
			pixel (x1 >> 16, y1, colidx);
			x1 += dx;
			y1++;
		}
	} else {
		if (x1 > x2) {
			tmp = x1; x1 = x2; x2 = tmp;
			tmp = y1; y1 = y2; y2 = tmp;
			dx = -dx; dy = -dy;
		}
		y1 <<= 16;
		dy = dx ? (dy << 16) / dx : 0;
		while (x1 <= x2) {
			pixel (x1, y1 >> 16, colidx);
			y1 += dy;
			x1++;
		}
	}
}

void rect (int x1, int y1, int x2, int y2, unsigned colidx)
{
	line (x1, y1, x2, y1, colidx);
	line (x2, y1, x2, y2, colidx);
	line (x2, y2, x1, y2, colidx);
	line (x1, y2, x1, y1, colidx);
}

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

static void
Init(rtems_task_argument arg)
{
	rtems_status_code sc;
	int exit_code;

	(void)arg;

	puts("\nRTEMS I2C TEST\n");
	exit_code = bbb_register_i2c_0();
	assert(exit_code == 0);
	sc = rtems_bsd_initialize();
	assert(sc == RTEMS_SUCCESSFUL);
	exit_code = open_framebuffer();
	assert(exit_code == 0);
	setcolor (4, 0xff0000);
	rect(100, 200, 200, 300, 4);
	exit_code = close_framebuffer();
	/* Some time for USB device to be detected. */
//	rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(4000));
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
  &rtems_shell_I2C_Command

#define CONFIGURE_SHELL_COMMANDS_ALL

#include <rtems/shellconfig.h>
