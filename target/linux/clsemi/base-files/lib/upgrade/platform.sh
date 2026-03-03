REQUIRE_IMAGE_METADATA=1

RAMFS_COPY_BIN='/sbin/checkclsimg /usr/sbin/fw_setenv /bin/gunzip /etc/openwrt_cls_info'

get_version() {
    local file=$1
    #remove V or v in version number
    version=$(awk '{print $2}' "$file" | busybox tr -d 'vV')
    echo "$version"
}

function version_lt() {
	test "$(echo "$@" | busybox tr " " "\n" | busybox sort -rV | busybox head -n 1)" != "$1";
}


platform_check_image() {
	checkclsimg -c $1 1>/dev/null
	if [ $? -ne 0 ];then
		echo "error"
		return 1
	fi
	checkclsimg -x $1 1>/dev/null

	versionNew=$(get_version "/tmp/split_img_dir/image-info")
	versionOld=$(get_version "/etc/openwrt_cls_info")

	if [ "$versionNew" != "22.03.2" ] && [ "$versionOld" != "22.03.2" ]; then
		if version_lt $versionNew $versionOld; then
			echo "New:$versionNew is less than Old:$versionOld"
			return 2;
		fi
	fi

	return 0
}

platform_do_upgrade() {
	echo "start upgrade cls image"
	rm /tmp/split_img_dir/ -rf
	checkclsimg $1
	echo "upgrade cls image done"
}
