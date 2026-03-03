#!/bin/bash
echo "perpare enter into ddr diagstest mode..."

#Switch to DDR Diagstest
devmem 0x90444420 32 0x5a5aa5a5
devmem 0x90444424 32 0x44445254

#Triggle Goable Rest
devmem 0x90444200 32 0xA5A5A5A5
devmem 0x90444200 32 0x765A765A

