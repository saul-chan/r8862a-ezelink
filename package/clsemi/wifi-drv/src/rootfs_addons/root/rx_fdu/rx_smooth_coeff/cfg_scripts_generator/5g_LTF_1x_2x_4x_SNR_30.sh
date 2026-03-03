#!/bin/bash
reg_addr=0x4AC08C38

smo_coff_1x=(
     466	101	-42	-39	7	28	12	-14	-18	3	19	-9
     100	257	182	38	-50	-48	2	35	21	-17	-28	19
     -84	366	460	301	75	-63	-67	1	45	18	-33	6
     -79	77	302	399	311	118	-43	-87	-27	45	43	-36
     14	-100	75	310	409	313	106	-60	-87	1	71	-28
     56	-96	-63	118	311	384	291	106	-43	-67	4	23

)

smo_coff_2x=(
     389	171	34	-34	-49	-33	-5	18	26	18	-1	-21
     338	322	249	150	55	-15	-49	-47	-19	14	28	-2
     66	248	304	266	175	72	-13	-58	-59	-28	14	36
     -67	151	267	292	248	163	68	-13	-58	-60	-19	52
     -98	55	177	249	266	231	158	68	-13	-58	-47	35
     -131	-31	145	329	464	510	455	317	136	-25	-99	-21
)

smo_coff_4x=(
     300	185	96	31	-12	-35	-41	-35	-21	-2	16	30
     369	287	209	138	76	26	-10	-31	-38	-29	-5	32
     382	417	407	361	290	204	114	33	-30	-63	-58	-8
     122	275	361	389	369	311	229	135	45	-30	-76	-82
     -47	152	290	369	394	374	317	234	135	33	-63	-140
     -139	53	203	311	374	395	374	317	229	114	-19	-164
)

coff_shift_1x=(
    0
    0
    3
    3
    3
    3
)

coff_shift_2x=(
    0
    3
    3
    3
    3
    2
)

coff_shift_4x=(
    0
    3
    2
    2
    2
    2
)

mask_0=$((0x3ff))
mask_1=$((0x3ff << 10))
mask_2=$((0x3ff << 20))
mask_3=$((0x3 << 30))

for k in $(seq 0 2)
do
    if [ $k -eq 0 ]; then
        smo_coff=("${smo_coff_1x[@]}")
        coff_shift=("${coff_shift_1x[@]}")
        printf "#1x reg config\n\n"
        printf "echo \"#1x reg config\"\n"
    fi
    if [ $k -eq 1 ]; then
        smo_coff=("${smo_coff_2x[@]}")
        coff_shift=("${coff_shift_2x[@]}")
        printf "#2x reg config\n\n"
        printf "echo \"#2x reg config\"\n"
    fi
    if [ $k -eq 2 ]; then
        smo_coff=("${smo_coff_4x[@]}")
        coff_shift=("${coff_shift_4x[@]}")
        printf "#4x reg config\n\n"
        printf "echo \"#4x reg config\"\n"
    fi

    for i in $(seq 0 5)
    do
        for j in $(seq 0 3)
        do
            # echo "i = $i, j = $j"
            printf "\n"
            printf "echo;\n"
            val2reg=0

            coff_id_0=$((i * 12 + (3 - j) * 3 + 2))
            # echo "coff_id_0 = $coff_id_0"
            coff_id_1=$((i * 12 + (3 - j) * 3 + 1))
            # echo "coff_id_1 = $coff_id_1"
            coff_id_2=$((i * 12 + (3 - j) * 3 + 0))
            # echo "coff_id_2 = $coff_id_2"

            val_0=$((smo_coff[$coff_id_0]))
            # printf "val_0: %d\n" $val_0
            # printf "Hex val_0: 0x%x\n" $val_0
            val_0=$((val_0 & mask_0))
            # printf "Hex val_0: 0x%x\n" $val_0

            val_1=$((smo_coff[$coff_id_1]))
            # printf "val_1: %d\n" $val_1
            # printf "Hex val_1: 0x%x\n" $val_1
            val_1=$((val_1 << 10))
            # printf "Hex val_1: 0x%x\n" $val_1
            val_1=$((val_1 & mask_1))
            # printf "Hex val_1: 0x%x\n" $val_1

            val_2=$((smo_coff[$coff_id_2]))
            # printf "val_2: %d\n" $val_2
            # printf "Hex val_2: 0x%x\n" $val_2
            val_2=$((val_2 << 20))
            # printf "Hex val_2: 0x%x\n" $val_2
            val_2=$((val_2 & mask_2))
            # printf "Hex val_2: 0x%x\n" $val_2

            val_3=0
            if [ $j -eq 3 ]; then
                # echo "jump in"
                val_3=$((coff_shift[$i]))
                # printf "val_3: 0x%x\n" $val_3
                val_3=$((val_3 << 30))
                # printf "Hex val_3: 0x%x\n" $val_3
                val_3=$((val_3 & mask_3))
                # printf "Hex val_3: 0x%x\n" $val_3
            fi

            val2reg=$(($val_0 | $val_1 | $val_2 | $val_3))

            #get reg value
            printf "echo \"devmem 0x%x 32\"\n" $reg_addr
            printf "devmem 0x%x 32\n" $reg_addr
            # devmem $reg_addr 32
            # sleep 1

            #set reg value
            printf "echo \"devmem 0x%x 32 0x%x\"\n" $reg_addr $val2reg
            printf "devmem 0x%x 32 0x%x\n" $reg_addr $val2reg
            # devmem $reg_addr 32 $val2reg
            # sleep 1

            #get reg value again
            printf "echo \"devmem 0x%x 32\"\n" $reg_addr
            printf "devmem 0x%x 32\n" $reg_addr
            # devmem $reg_addr 32
            # sleep 1

            reg_addr=$(($reg_addr + 4))
        done
    done
    printf "\n\n\n"
done
