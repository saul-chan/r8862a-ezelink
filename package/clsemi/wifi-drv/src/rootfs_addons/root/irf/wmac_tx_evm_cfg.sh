#!/bin/bash

radio=$1
bw=$2
mpdu_subframe=$3
mpdu_len=$4
mcs=$5
tx_intv=$6
ppdu_type=$7
gi=$8
ack_policy=$9
channel=${10}
he_ltf=4

function help {
    echo "Usage: sh wmac_tx_evm_cfg.sh <radio> <bw> <mpdu_subframe> <mpdu_len> <mcs> <tx_intv> <ppdu_type> <gi> <ack_policy> <he_ltf>"
    echo "radio: 2G or 5G"
    echo "bw: 20, 40, 80, 160"
    echo "mpdu_subframe: subframe number"
    echo "mpdu_len: mpdu length number"
    echo "mcs: 0xXY X-spatial stream Y-mcs rate"
    echo "tx_intv: tx interval"
    echo "ppdu_type: HT-MF, VHT, HE-SU"
    echo "gi: long, short, 0.8, 1.6, 3.2"
    echo "ack_policy: 0:normal ack, 1:no ack, 2:no explicit ack, 3:block ack"
    echo "channel: target channel"
    echo "he_ltf: 1, 2, 4"
}

if [ $# -ne 10 -a $# -ne 11 ]; then
    help
    return
fi

if [ $# -eq 11 ]; then
   he_ltf=${11}
fi

set -x

calcmd set radio $radio
calcmd log module 7 level 0
calcmd set bss bw $bw tx-ppdu-cnt 0 tx-interval $tx_intv channel $channel
if [ "$ppdu_type" == "HE-SU" ]; then
    calcmd set mac-phy ppdu-type $ppdu_type gi $gi he-ltf $he_ltf stbc 0 bw-ppdu $bw
else
    calcmd set mac-phy ppdu-type $ppdu_type gi $gi he-ltf 2 stbc 0 bw-ppdu $bw
fi
if [ "$ppdu_type" == "HE-SU" ]; then
    calcmd set su mac-addr 00:55:55:55:55:00 mpdu-type 2 mpdu-subtype 8 mpdu-ackpolicy $ack_policy mpdu-body 0x5a mpdu-body-len $mpdu_len mpdu-subframe $mpdu_subframe mcs $mcs fec 1 mpdu-fc-flags 0x2 packet-ext 16
else
    calcmd set su mac-addr 00:55:55:55:55:00 mpdu-type 2 mpdu-subtype 8 mpdu-ackpolicy $ack_policy mpdu-body 0x5a mpdu-body-len $mpdu_len mpdu-subframe $mpdu_subframe mcs $mcs fec 1 mpdu-fc-flags 0x2
fi
calcmd update

echo "wmac cali configuration done!"
