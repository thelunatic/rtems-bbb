define reset
	print "Reset target and wait for U-Boot to start kernel."
	monitor reset
	# RTEMS U-Boot starts at this address.
	tbreak *0x80000000
	# Linux starts here.
	tbreak *0x82000000
	continue

	print "Overwrite kernel with application to debug."
	load
end

target remote :2331
