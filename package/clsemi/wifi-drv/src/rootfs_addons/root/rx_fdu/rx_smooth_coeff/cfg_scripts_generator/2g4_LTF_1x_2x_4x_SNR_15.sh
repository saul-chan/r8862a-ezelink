#!/bin/bash
reg_addr=0x48C08C38

smo_coff_1x=(
     392	168	12	-51	-41	-1	28	25	0	-21	-17	17
     320	377	289	132	-7	-71	-56	-2	38	36	-1	-32
     23	287	375	300	137	-12	-81	-59	6	52	35	-40
     -97	132	302	350	275	131	-1	-65	-49	6	38	0
     -79	-7	141	279	333	278	148	12	-66	-61	-2	48
     -1	-72	-13	134	281	344	290	149	-1	-83	-57	54
)

smo_coff_2x=(
     293   183	91	23	-20	-38	-37	-23	-3	13	19	11
     358	295	221	145	74	17	-22	-41	-41	-24	4	37
     353	438	454	408	316	198	78	-22	-83	-93	-47	49
     88	286	407	447	414	325	204	78	-25	-83	-81	-13
     -77	148	316	415	442	404	315	199	78	-22	-82	-88
     -149  34	198	326	405	427	395	316	205	78	-44	-143
)

smo_coff_4x=(
     445	333	231	141	67	8	-33	-56	-62	-50	-21	22
     324	264	206	151	102	59	24	-3	-21	-30	-30	-21
     441	403	358	308	254	197	141	85	32	-16	-59	-95
     266	291	303	301	284	255	213	161	100	32	-41	-116
     124	194	247	281	296	290	265	220	159	83	-6	-105
     15	112	191	251	289	303	295	263	210	136	45	-61
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
