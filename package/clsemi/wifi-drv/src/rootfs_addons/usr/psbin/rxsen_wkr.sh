set -x
phy=$1


if [ $phy -eq 1 ]; then
devmem 0x4010b390
devmem 0x4010b390 32 0x00001103
devmem 0x4010b390
devmem 0x4010b390 32 0x00000103
else
devmem 0x4030b390
# devmem 0x4030b390 32 0x00001003
# devmem 0x4030b390 32 0x00000003
fi




