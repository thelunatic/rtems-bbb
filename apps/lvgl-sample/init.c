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

#include "lvgl/lvgl.h"

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

static size_t fbsize;
static unsigned char *fbuffer;
static unsigned char **line_addr;
static int fb_fd=0;
static int bytes_per_pixel;
unsigned xres, yres;

static char *defaultfbdevice = "/dev/fb0";
static char *fbdevice = NULL;

int open_framebuffer(void)
{
	int y;
	unsigned addr;
	struct fbtype fb;
	unsigned line_length;

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

	xres = (unsigned) fb.fb_width;
	yres = (unsigned) fb.fb_height;

	unsigned pagemask = (unsigned) getpagesize() - 1;
	fbsize = ((line_length*yres + pagemask) & ~pagemask);

	fbuffer = (unsigned char *)mmap(0, fbsize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (fbuffer == (unsigned char *)-1) {
		perror("mmap framebuffer");
		close(fb_fd);
		return -1;
	}
	memset(fbuffer,0, fbsize);

	bytes_per_pixel = (fb.fb_depth + 7) / 8;
	line_addr = malloc (sizeof (char*) * yres);
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

void fbdev_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(fbuffer == NULL ||
            area->x2 < 0 ||
            area->y2 < 0 ||
            area->x1 > (int32_t)xres - 1 ||
            area->y1 > (int32_t)yres - 1) {
        lv_disp_flush_ready(drv);
        return;
    }

    /*Truncate the area to the screen*/
    int32_t act_x1 = area->x1 < 0 ? 0 : area->x1;
    int32_t act_y1 = area->y1 < 0 ? 0 : area->y1;
    int32_t act_x2 = area->x2 > (int32_t)xres - 1 ? (int32_t)xres - 1 : area->x2;
    int32_t act_y2 = area->y2 > (int32_t)yres - 1 ? (int32_t)yres - 1 : area->y2;


    lv_coord_t w = lv_area_get_width(area);
    unsigned char *location;
    long int byte_location = 0;
    unsigned char bit_location = 0;

	/* ASSUMING 16bit */
        int32_t y,x;
        for(y = act_y1; y <= act_y2; y++) {
			for( x = act_x1; x<= act_x2; x++){
            	location = x * bytes_per_pixel + line_addr[y];
				*location = lv_color_to16(*color_p);
				color_p++ ;
			}
		}

    lv_disp_flush_ready(drv);
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
	static lv_color_t buf[LV_HOR_RES_MAX*10];
	static lv_disp_buf_t disp_buf;

	puts("\nRTEMS I2C TEST\n");
	exit_code = bbb_register_i2c_0();
	assert(exit_code == 0);
	sc = rtems_bsd_initialize();
	assert(sc == RTEMS_SUCCESSFUL);

	lv_init();

	exit_code = open_framebuffer();
	assert(exit_code == 0);
	
	lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX*10);

	lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    lv_disp_drv_register(&disp_drv);

	lv_obj_t * label = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(label, "Hello world!");
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);

    lv_tick_inc(5);
    lv_task_handler();

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
