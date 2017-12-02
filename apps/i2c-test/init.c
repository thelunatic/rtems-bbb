/*
 * Copyright (c) 2017 Christian Mauderer <oss@c-mauderer.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <rtems.h>
#include <bsp/i2c.h>
#include <dev/i2c/eeprom.h>

#define EEPROM_I2C_ADDR 0x50
#define EEPROM_ADDRESS_SIZE 2
#define EEPROM_PAGE_SIZE 32
#define EEPROM_SIZE 4096
#define EEPROM_PROG_TIMEOUT_MS 2000
#define EEPROM_PATH "/dev/i2c-0.eeprom"

#define BYTES_PER_LINE 32

static bool
register_bus(void)
{
	int rv = bbb_register_i2c_0();
	return (rv == 0);
}

static bool
register_eeprom(void)
{
	int rv = i2c_dev_register_eeprom(
	    BBB_I2C_0_BUS_PATH, EEPROM_PATH, EEPROM_I2C_ADDR,
	    EEPROM_ADDRESS_SIZE, EEPROM_PAGE_SIZE, EEPROM_SIZE,
	    EEPROM_PROG_TIMEOUT_MS);
	return (rv == 0);
}

static bool
do_hex_dump(void)
{
	bool ok = true;
	int fd;
	static char buffer[EEPROM_SIZE];
	ssize_t bytes_read;
	ssize_t i;

	if (ok) {
		fd = open(EEPROM_PATH, O_RDONLY);
		if (fd == -1) {
			perror("Could not open eeprom file");
			ok = false;
		}
	}

	if (ok) {
		bytes_read = read(fd, buffer, sizeof(buffer));

		if (bytes_read < 0) {
			perror("Problems while reading");
			ok = false;
		} else {
			for (i = 0; i < bytes_read; ++i) {
				printf("%02X ", buffer[i]);
				if ((i+1) % BYTES_PER_LINE == 0) {
					puts("");
				}
			}
		}
	}

	if (fd != -1) {
		close(fd);
	}

	return ok;
}

static void
Init(rtems_task_argument arg)
{
	bool ok;

	(void)arg;
	ok = register_bus();
	assert(ok);
	ok = register_eeprom();
	assert(ok);
	ok = do_hex_dump();

	sleep(5);
}

#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

#define CONFIGURE_LIBIO_MAXIMUM_FILE_DESCRIPTORS 10
#define CONFIGURE_UNLIMITED_OBJECTS
#define CONFIGURE_UNIFIED_WORK_AREAS

#define CONFIGURE_USE_IMFS_AS_BASE_FILESYSTEM

#define CONFIGURE_RTEMS_INIT_TASKS_TABLE 
#define CONFIGURE_INIT

#include <rtems/confdefs.h>
