#!/bin/bash
reg_addr=0x48C08C38

smo_coff_1x=(
     360	187	42	-38	-50	-20	15	28	15	-8	-18	-1
     338	355	271	136	10	-58	-60	-17	27	41	14	-33
     74	263	333	278	147	13	-64	-65	-15	35	39	-14
     -67	134	282	326	265	141	17	-55	-58	-15	27	27
     -91	11	154	274	318	269	154	27	-57	-68	-18	52
     -38	-61	14	149	276	330	282	157	18	-69	-62	28
)

smo_coff_2x=(
     511	359	216	95	4	-54	-78	-73	-46	-8	32	65
     345	290	223	152	85	27	-15	-38	-43	-30	-4	31
     407	436	422	370	290	195	98	15	-43	-69	-59	-14
     176	293	365	388	365	304	217	120	29	-43	-83	-86
     7	162	284	363	393	374	314	224	119	15	-74	-134
     -99	52	191	301	373	399	378	313	215	96	-28	-144
)

smo_coff_4x=(
     370	298	229	165	107	56	14	-19	-41	-53	-55	-47
     286	244	202	160	120	82	47	17	-9	-29	-44	-53
     424	390	350	305	255	203	149	94	41	-9	-56	-98
     297	302	297	283	260	229	190	145	95	40	-16	-74
     378	444	489	511	511	487	441	373	285	181	63	-66
     196	301	385	446	483	494	478	437	371	282	174	49
)

coff_shift_1x=(
    0
    3
    3
    3
    3
    3
)

coff_shift_2x=(
    3
    3
    2
    2
    2
    2
)

coff_shift_4x=(
    3
    3
    2
    2
    1
    1
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
