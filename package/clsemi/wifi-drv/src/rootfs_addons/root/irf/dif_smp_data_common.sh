#!/bin/bash

#SRC ST5 采数,channel 0
sh /root/irf/irf_smp_config.sh 0 0xc808 0xc000 65536 0 0 0 0 0x10 0
sh /root/irf/irf_smp_start.sh 0x1 0 0
sleep 5

#AGC status address valid, 128 bit width, hardware trigger: state address valid, 80M rate, high level
sh /root/irf/irf_smp_config.sh 3 0xd850 0xd80d 16384 2 0 3 1 0x10 0 0

sh /root/irf/irf_smp_start.sh 0x8 0 0

#AGC status rx_on, 128 bit width, hardware trigger: state address valid, 80M rate, high level
sh /root/irf/irf_smp_config.sh 3 0xd84f 0xd80d 16384 2 0 3 1 0x10 0 0

sh /root/irf/irf_smp_start.sh 0x8 0 0

#AGC AC/CC/Ref, 128 bit width, hardware trigger: state address valid, 80M rate, high level
sh /root/irf/irf_smp_config.sh 3 0xd84b 0xd80d 16384 2 0 3 1 0x10 0 0

sh /root/irf/irf_smp_start.sh 0x8 0 0

#AGC DSPEN/Event, 128 bit width, hardware trigger: state address valid, 80M rate, high level
sh /root/irf/irf_smp_config.sh 3 0xd849 0xd80d 16384 2 0 3 1 0x10 0 0

sh /root/irf/irf_smp_start.sh 0x8 0 0

#AGC CCA, 128 bit width, hardware trigger: state address valid, 80M rate, high level
sh /root/irf/irf_smp_config.sh 3 0xd847 0xd80d 16384 2 0 3 1 0x10 0 0

sh /root/irf/irf_smp_start.sh 0x8 0 0