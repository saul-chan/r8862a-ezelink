echo -e "123456\r123456\r" | passwd root
rm /dev/null
mknod /dev/null c 1 3
/etc/init.d/S50dropbear start
