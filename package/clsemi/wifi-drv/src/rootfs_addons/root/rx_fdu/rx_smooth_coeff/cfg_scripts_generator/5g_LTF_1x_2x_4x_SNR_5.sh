#!/bin/bash
reg_addr=0x4AC08C38

smo_coff_1x=(
     318	198	75	-14	-49	-39	-6	22	28	12	-10	-22
     337	339	261	140	24	-47	-59	-28	14	36	23	-17
     120	247	294	252	149	36	-42	-62	-34	9	34	20
     -23	135	256	299	254	151	36	-43	-64	-35	13	45
     -85	25	160	268	305	261	156	38	-45	-66	-29	37
     -70	-49	41	165	272	313	270	163	40	-46	-62	-11
)

smo_coff_2x=(
     451	347	238	135	46	-20	-61	-75	-64	-35	7	54
     322	280	225	163	102	47	2	-27	-40	-37	-20	7
     419	426	402	353	283	203	120	44	-16	-55	-70	-61
     228	298	340	351	331	284	217	139	58	-15	-73	-109
     77	181	266	323	346	335	292	222	135	42	-48	-123
     -33	82	188	274	331	353	338	288	210	111	4	-99
)

smo_coff_4x=(
     304	261	217	173	129	88	48	13	-18	-45	-66	-81
     494	441	382	320	257	193	129	69	12	-40	-85	-124
     393	366	332	294	251	204	156	106	57	8	-38	-81
     303	297	284	266	241	211	177	138	97	55	11	-32
     443	466	475	472	455	426	385	333	271	201	125	44
     297	345	383	409	422	421	407	381	342	292	232	164
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
    2
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
