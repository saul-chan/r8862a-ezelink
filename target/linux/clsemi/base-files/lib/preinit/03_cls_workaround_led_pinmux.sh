cls_workaround_led_pinmux() {
	local board=$(board_name)

	case "$board" in
	clsemi,dubhe1000-rdk)
		# Just set the pinmux to workaround 5G LED is not in GPIO mode
		devmem 0x90420064 32 0x5008
		;;
	esac
}

boot_hook_add preinit_main cls_workaround_led_pinmux
