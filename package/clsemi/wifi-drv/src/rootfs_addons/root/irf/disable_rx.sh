#!/bin/bash
devmem 0x90422120 32 0x70090000
sleep 1
devmem 0x9042212C 32 0xFE380000
sleep 1
devmem 0x904221AC 32 0xA0492490

