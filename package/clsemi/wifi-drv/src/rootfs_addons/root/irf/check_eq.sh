#!/bin/bash

cd /mnt/ubifs_data/m2k

if [ -e eq_cali_xtal_ctrim.bin ]; then
    echo "xtal check successful!"
else
    echo "xtal check failed!"
fi

if [ -e dif_eq_5G.bin ]; then
    echo "DIF 5G check successful!"
else
    echo "DIF 5G check failed!"
fi

if [ -e dif_eq_2G.bin ]; then
    echo "DIF 2G check successful!"
else
    echo "DIF 2G check failed!"
fi

if [ -e tx_gain_fcomp_5G.bin ]; then
    echo "TX gain fcomp 5G check successful!"
else
    echo "TX gain fcomp 5G check failed!"
fi

if [ -e tx_gain_fcomp_2G.bin ]; then
    echo "TX gain fcomp 2G check successful!"
else
    echo "TX gain fcomp 2G check failed!"
fi

if [ -e rx_gain_fcomp_5G.bin ]; then
    echo "RX gain fcomp 5G check successful!"
else
    echo "RX gain fcomp 5G check failed!"
fi

if [ -e rx_gain_fcomp_2G.bin ]; then
    echo "RX gain fcomp 2G check successful!"
else
    echo "RX gain fcomp 2G check failed!"
fi

if [ -e fb_gain_fcomp_5G.bin ]; then
    echo "FB gain fcomp 5G check successful!"
else
    echo "FB gain fcomp 5G check failed!"
fi

if [ -e fb_gain_fcomp_2G.bin ]; then
    echo "FB gain fcomp 2G check successful!"
else
    echo "FB gain fcomp 2G check failed!"
fi

if [ -e rx_dcoc_5G_low.bin -o rx_dcoc_5G_high.bin ]; then
    echo "RX DCOC 5G check successful!"
else
    echo "RX DCOC 5G check failed!"
fi

if [ -e rx_dcoc_2G_low.bin -o -e rx_dcoc_2G_high.bin ]; then
    echo "RX DCOC 2G check successful!"
else
    echo "RX DCOC 2G check failed!"
fi


cd - > /dev/null
