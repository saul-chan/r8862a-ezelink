attach_ubi_mtd() {
	local mtdnum=$(find_mtd_index $1)

	[ -z "$mtdnum" ] && return
	ubiattach /dev/ubi_ctrl -m $mtdnum
}

cls_attach_env_sys() {
	local board=$(board_name)

	case "$board" in
	clourney,dubhe2000-emu|\
	clourney,dubhe2000|\
	clsemi,dubhe2000-evb-a|\
	clsemi,dubhe2000-evb-b|\
	clsemi,dubhe2000-demo-o|\
	clsemi,dubhe2000-demo-r|\
	clsemi,dubhe2000-demo-a|\
	clsemi,dubhe2000-demo-b|\
	clsemi,dubhe2000-rdk|\
	clsemi,dubhe2000-rdk-s|\
	clsemi,triband-5400-rdk)
		attach_ubi_mtd "ubootenv"
		attach_ubi_mtd "boarddata"
		;;
	clsemi,dubhe2000-evb-e)
		;;
	*)
		echo "Error: ubootenv and boarddata are not attached for board $board" > /dev/console
		;;
	esac
}

boot_hook_add preinit_main cls_attach_env_sys
