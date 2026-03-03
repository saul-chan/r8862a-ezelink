#!/bin/bash
# channel 0
devmem 0x40280968 32 1
devmem 0x4028096c 32 0xa155a155
#devmem 0x40280970 32 0xa155
# channel 1
devmem 0x40288968 32 1
devmem 0x4028896c 32 0xa155a155
#devmem 0x40288970 32 0xa155
echo "CFR SW bypass done!"
