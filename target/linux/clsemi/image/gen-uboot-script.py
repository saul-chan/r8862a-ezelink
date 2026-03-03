#!/usr/bin/python3
import sys
import zlib
import os

def usage_and_exit():
    print("Usage: " + sys.argv[0] + " <script_output> <initramfs | squashfs> [kernel_img] [rootfs_img] ")
    exit(1)

if len(sys.argv) < 2:
    usage_and_exit()

script_file = sys.argv[1]

if sys.argv[2] == "initramfs":
    with open(script_file, "w") as fout:
        print("echo boardtype: $boardtype",
              "echo ramdisk img for $boardtype ...",
              "setenv bootargs console=ttyS0,115200 rdinit=/init rw rootwait",
              file=fout, sep="\n")
        with open("supported_devices", "r") as devices:
            next(devices)
            count = 0
            for line in devices:
                if count == 0:
                    print("if test $boardtype = {}; then".format(line.split()[0]),
                           file=fout, sep="\n")
                else:
                    print("elif test $boardtype = {}; then".format(line.split()[0]),
                           file=fout, sep="\n")
                print("      bootm $load_k_addr#clsemi-{}".format(count),
                       file=fout, sep="\n")
                count += 1
        print("else",
        "      bootm $load_k_addr",
        "fi", file=fout, sep="\n")

elif sys.argv[2] == "squashfs":
    with open(script_file, "w") as fout:
            print("echo boot from $active_fw ...", file=fout, sep="\n")
            with open("supported_devices", "r") as devices:
                next(devices)
                count = 0
                for line in devices:
                    if count == 0:
                        print("if test $boardtype = {}; then".format(line.split()[0]),
                               file=fout, sep="\n")
                    else:
                        print("elif test $boardtype = {}; then".format(line.split()[0]),
                               file=fout, sep="\n")
                    print("      setenv bootmcmd bootm $load_k_addr#clsemi-{}".format(count),
                          "      setenv fw_flash_type {}".format(line.split()[2]),
                           file=fout, sep="\n")
                    count += 1
            print("else",
            "      setenv bootmcmd bootm $load_k_addr",
            "      setenv fw_flash_type nand ",
            "fi",
            "if test $fw_flash_type = nand; then",
            "        setenv bootargs ubi.mtd=${active_fw} rootfstype=squashfs root=/dev/ubiblock0_0p1 rootwait ro",
            "elif test $fw_flash_type = emmc; then",
            "        setenv bootargs root=/dev/mmcblk0p65 rootwait cls_rootdisk=$active_fw",
            "else",
            "        echo FW flash type is $fw_flash_type and not supported",
            "        exit 1",
            "fi",
            "iminfo $load_k_addr",
            "if test $? = 0;then",
            "	run bootmcmd",
            "else",
            "	echo $active_fw is bad, switch to next fw",
                                "fi", file=fout, sep="\n")
else:
    usage_and_exit()
