#!/bin/bash
# channel 0
devmem 0x40080968 32 1
devmem 0x4008096c 32 0xa155a155
#devmem 0x4c810970 32 0xa155
# channel 1
devmem 0x40088968 32 1
devmem 0x4008896c 32 0xa155a155
#devmem 0x4c818970 32 0xa155
echo "CFR SW bypass done!"
