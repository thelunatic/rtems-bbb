define reload
	monitor reset
	monitor halt
	load
	monitor reset
end

define reset
	monitor reset
end

target remote :2331
