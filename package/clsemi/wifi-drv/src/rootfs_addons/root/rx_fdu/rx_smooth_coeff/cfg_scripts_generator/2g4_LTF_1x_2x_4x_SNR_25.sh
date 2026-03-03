#!/bin/bash
reg_addr=0x48C08C38

smo_coff_1x=(
     447	122	-31	-48	-7	25	20	-4	-18	-7	12	0
     240	462	345	103	-71	-98	-21	55	54	-13	-56	25
     -61	347	442	303	92	-51	-74	-16	37	31	-13	-13
     -95	105	305	380	299	127	-28	-84	-40	37	54	-36
     -13	-72	92	299	388	306	120	-41	-84	-16	55	-8
     49	-99	-51	126	305	369	288	120	-28	-74	-22	40
)

smo_coff_2x=(
     362	182	54	-20	-49	-44	-20	9	28	30	10	-30
     358	307	229	142	62	1	-34	-42	-29	-6	15	20
     106	229	270	246	179	93	12	-45	-68	-52	-6	59
     -40	142	247	277	245	172	81	-2	-57	-68	-29	55
     -97	63	181	247	258	222	153	71	-2	-45	-42	17
     -175	3	189	347	446	470	419	307	164	25	-68	-79
)

smo_coff_4x=(
     274	184	108	47	3	-27	-42	-44	-34	-14	13	45
     366	284	207	138	78	30	-6	-27	-35	-28	-8	25
     429	413	378	326	264	195	126	61	6	-35	-57	-57
     188	274	325	343	331	294	237	166	87	6	-70	-134
     10	155	263	331	362	357	319	254	166	61	-55	-173
     -107	59	194	294	356	381	368	319	237	125	-11	-167
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
