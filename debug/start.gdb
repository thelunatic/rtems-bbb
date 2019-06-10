define reset
	echo -- Reset target and wait for U-Boot to start kernel.\n
	monitor reset
	# RTEMS U-Boot starts at this address.
	tbreak *0x80000000
	# Linux starts here.
	tbreak *0x82000000
	continue

	echo -- Disable watchdog.\n
	set *(uint32_t*)0x44e35048=0xAAAA
	while (*(uint32_t*)0x44e35034 != 0)
	end
	set *(uint32_t*)0x44e35048=0x5555
	while (*(uint32_t*)0x44e35034 != 0)
	end

	echo -- Overwrite kernel with application to debug.\n
	load
end

# Hack for JLink problems:
# https://forum.segger.com/index.php/Thread/6140-JLinkGDBServer-issues-with-6-44d/
python
import os
gdb.execute('set tdesc filename ' + os.environ['PROJECTDIR'] + '/debug/tdesc.xml')
end

target remote :2331
