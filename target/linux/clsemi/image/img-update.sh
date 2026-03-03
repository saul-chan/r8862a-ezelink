#!/bin/sh
. /lib/functions.sh
. /lib/functions/system.sh
. /lib/upgrade/nand.sh
[ -f /lib/upgrade/emmc.sh ] && . /lib/upgrade/emmc.sh

get_version() {
    local file=$1
    #remove V or v in version number
    version=$(awk '{print $2}' "$file" | busybox tr -d 'vV')
    echo "$version"
}

function version_lt() { test "$(echo "$@" | busybox tr " " "\n" | busybox sort -rV | busybox head -n 1)" != "$1"; }

versionNew=$(get_version "/tmp/split_img_dir/image-info")
versionOld=$(get_version "/tmp/root/etc/openwrt_cls_info")

echo "versionNew=$versionNew"
echo "versionOld=$versionOld"

if [ "$versionNew" != "22.03.2" ] && [ "$versionOld" != "22.03.2" ]; then
	if version_lt $versionNew $versionOld; then
		echo "New:$versionNew is less than Old:$versionOld"
		#return 1;
	fi
fi


#partname is opposite partition
active_fw="$(fw_printenv -n active_fw)"
backup_fw_tmp="$(fw_printenv -n backup_fw)"
board_type="$(cat /tmp/sysinfo/board_name)"
find_all_part=0
find_bootloader=0

echo active_fw = $active_fw
echo backup_fw_tmp = $backup_fw_tmp

if [ "$backup_fw_tmp" == "" ]; then
	echo "no backup_fw set, set it to fw1"
	active_fw="fw2"
	#upgrade fw to fw1
	partname="fw1"
	backup_fw_tmp="fw1"
	backup_fw="fw1"
else
	#if have backup_fw set
	if [ "$active_fw" == "fw1" ]; then
		backup_fw="fw2"
	else
		backup_fw="fw1"
	fi
	partname=$backup_fw
fi

set_opposite_fw_to_active () {
	if [ "$backup_fw_tmp" == "" ]; then
		echo "no need to_opposite_fw"
	else
		echo "set_opposite_fw_to_active"
		fw_setenv active_fw $backup_fw_tmp
		fw_setenv backup_fw $active_fw
	fi
}

nand_write_partition() {
	[ "$1" = "" ] || [ "$2" = "" ] && return

	if [ -f "/tmp/split_img_dir/$1" ]; then
		echo "upgrade $1 to $2"
		mtd erase $2
		nandwrite $2 /tmp/split_img_dir/$1 -p -k
	fi
}

do_upgrade_failed() {
	sync
	echo "sysupgrade failed"
	# Should we reboot or bring up some failsafe mode instead?
	umount -a
	reboot -f
}

emmc_upgrade_fit() {
	local file="$1"

	dd if=$file of=$EMMC_KERN_DEV bs=512
}

emmc_do_flash_file() {
	local file="$1"

	export EMMC_KERN_DEV="$(find_mmc_part $CI_KERNPART $CI_ROOTDEV)"
	[ -z $EMMC_KERN_DEV ] && return 1
	emmc_upgrade_fit $file
}

emmc_do_restore_config() {
	export EMMC_DATA_DEV="$(find_mmc_part $CI_DATAPART $CI_ROOTDEV)"
	[ -z $EMMC_DATA_DEV ] && return 1
	if [ -z "$UPGRADE_BACKUP" ]; then
		dd if=/dev/zero of="$EMMC_DATA_DEV" bs=512 count=8
	else
		emmc_copy_config
	fi
}

emmc_do_upgrade_success() {
	if emmc_do_restore_config && set_opposite_fw_to_active && sync; then
		echo "sysupgrade successful"
		return 0
	fi
	emmc_do_upgrade_failed
}

emmc_do_upgrade_failed() {
	do_upgrade_failed
}

emmc_do_upgrade() {
	local file="$1"

	sync
	emmc_do_flash_file "$file"
	if [ $? = "0" ]; then
		emmc_do_upgrade_success
	else
		emmc_do_upgrade_failed
	fi
}

# Write the FIT image to UBI kernel volume
nand_upgrade_fit() {
	local fit_file="$1"

	local fit_length=$( (cat < "$fit_file" | wc -c) 2> /dev/null)

	nand_upgrade_prepare_ubi "" "" "$fit_length" "1" || return 1

	local fit_ubidev="$(nand_find_ubi "$CI_UBIPART")"
	local fit_ubivol="$(nand_find_volume $fit_ubidev "$CI_KERNPART")"
	cat < "$fit_file" | ubiupdatevol /dev/$fit_ubivol -s "$fit_length" -
}

nand_do_flash_file() {
	local file="$1"

	[ ! "$(find_mtd_index "$CI_UBIPART")" ] && return 1

	nand_upgrade_fit "$file"
}

nand_do_restore_config() {
	[ ! -f "$UPGRADE_BACKUP" ] || nand_restore_config "$UPGRADE_BACKUP"
}

nand_do_upgrade_success() {
	if nand_do_restore_config && set_opposite_fw_to_active && sync; then
		echo "sysupgrade successful"
		return 0
	fi
	nand_do_upgrade_failed
}

nand_do_upgrade_failed() {
	do_upgrade_failed
}

nand_do_upgrade() {
	local file="$1"

	sync
	nand_do_flash_file "$file"
	if [ $? = "0" ]; then
		nand_do_upgrade_success
	else
		nand_do_upgrade_failed
	fi
}

if [ -f "/tmp/split_img_dir/firmware" ]; then
	#only upgrade firmware to opposite fw partition
	echo "upgrade firmware to $partname [doing]"

	fw_file=/tmp/split_img_dir/firmware
	rootdev="$(cmdline_get_var root)"
	rootdev="${rootdev##*/}"
	rootdev="${rootdev%p[0-9]*}"
	case "$rootdev" in
	mmc*)
		CI_ROOTDEV=$rootdev
		CI_KERNPART=$partname
		CI_DATAPART="rootfs_data"
		emmc_do_upgrade $fw_file
		;;
	mtdblock*)
		# Add support when dev board which stores FW in nor flash is ready
		true
		;;
	ubiblock* | "")
		CI_UBIPART=$partname
		CI_KERNPART="kernel"
		nand_do_upgrade $fw_file
		;;
	esac
fi

echo "upgrade firmware [done]"
