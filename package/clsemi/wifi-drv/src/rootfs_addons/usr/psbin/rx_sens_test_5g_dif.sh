set -x
ant=$1

devmem 0x4010b004 32 $ant
sleep 1
devmem 0x4ac0882c 32 0x0004D03D
sleep 1
devmem 0x4ac08830 32 0x05409049
sleep 1
devmem 0x4010b36c 32 0x05200720
sleep 1
devmem 0x4010b304 32 0x42004200
sleep 1
devmem 0x4010b194 32 0x00011100
sleep 1
devmem 0x4010e654 32 0x1
sleep 1
devmem 0x4010b3C0 32 0x2828B080
sleep 1
devmem 0x4010b388 32 0x6403A809
sleep 1
devmem 0x4010b38C 32 0x04FFC40C
sleep 1
devmem 0x4ac08860 32 0x10D800F0
sleep 1
devmem 0x4010b390 32 0x00000103
