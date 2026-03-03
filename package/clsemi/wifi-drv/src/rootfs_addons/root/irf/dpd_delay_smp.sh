#!/bin/bash
cd /tmp/
rm *.dat
sh /root/irf/irf_smp_config.sh 2 24842 0x6300 4096 0 2 0 1 0x10 0
sleep 1
sh /root/irf/irf_smp_config.sh 3 24843 0x6300 4096 0 2 0 1 0x10 0
sleep 1
sh /root/irf/irf_smp_start.sh 0xc 10000 0
