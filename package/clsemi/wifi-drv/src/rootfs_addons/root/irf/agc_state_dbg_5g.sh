#!/bin/sh
agc_state0=$1
agc_state1=$2
jump_vld1=$3
jump_vld2=$4
jump_vld3=$5
#comb_agc_state="$agc_state0 $agc_state1"
echo $agc_state0
echo $agc_state1
#echo $comb_agc_state

state0_val=`grep -Ii "^\<$agc_state0\>" /root/irf/agcram_D2K_5G_v2.4_FEM_state_info.txt | awk -F ' ' '{printf("%s\n", $2);}'`
state1_val=`grep -Ii "^\<$agc_state1\>" /root/irf/agcram_D2K_5G_v2.4_FEM_state_info.txt | awk -F ' ' '{printf("%s\n", $2);}'`
jump_val=`grep -Ii "^\<$agc_state0\> \<$agc_state1\>" /root/irf/agcram_D2K_5G_v2.4_FEM_op_new_info.txt | awk -F ' ' '{printf("%s\n", $3);}'`

#echo $state0_val
#echo $state1_val
#echo $jump_val

reg_comb_val=`expr $state1_val \* 512 + $state0_val`
hex_reg_comb_val=0x`printf '%x' $reg_comb_val`
#echo $hex_reg_comb_val
devmem 0x4010e6fc 32 $hex_reg_comb_val
#devmem 0x4ac0e018 32 $hex_reg_comb_val

jump_val_adj=`printf '%d' $jump_val`
#echo $jump_val_adj

jump_comb_val=`expr $jump_vld3 \* 16777216 + $jump_vld2 \* 8388608 + $jump_vld1 \* 4194304 + $jump_val_adj`
hex_jump_comb_val=0x`printf '%x' $jump_comb_val`
#echo $hex_jump_comb_val
devmem 0x4010e6d8 32 $hex_jump_comb_val
#devmem 0x4ac0e020 32 $hex_jump_comb_val
