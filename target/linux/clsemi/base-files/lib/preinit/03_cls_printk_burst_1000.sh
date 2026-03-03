printk_ratelimit_burst_1000 () {
	#limit printk rate to 500 per 1 second
	echo 1 >  /proc/sys/kernel/printk_ratelimit
	echo 500 >  /proc/sys/kernel/printk_ratelimit_burst
}

boot_hook_add preinit_main printk_ratelimit_burst_1000

