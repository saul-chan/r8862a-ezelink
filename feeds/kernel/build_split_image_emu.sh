#!/bin/bash
export PATH=/opt/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/:$PATH

kernel_blcok=128
FitImage_blcok=32
dtb_blcok=464
ENTRY_ADDR=0x60800000
FDT_ADDR=0x61d00000
MakeMode="all"
KernelCompress="gzip"
kernel_dtb="clourney-dubhe1000-emu.dtb"
INITRD=""
ROOTFS=""
InitrdCompress="none"

while test $# -gt 0
do
    case "$1" in
        all*)
                MakeMode="all"
            ;;
        only*)
                MakeMode="only"
            ;;
        -C:*)   KernelCompress=( ${1#-C:} )
		        echo "Kernel Compress Mode $KernelCompress"
            ;;
        -c:*)   InitrdCompress=( ${1#-c:} )
		        echo "initrd/initramfs Compress Mode $InitrdCompress"
            ;;
        -e:*)   ENTRY_ADDR=( ${1#-e:} )
		        echo "Kernel Image Entry Address $ENTRY_ADDR"
            ;;
        -t:*)   FDT_ADDR=( ${1#-t:} )
		        echo "FDT $kernel_dtb Loading Address $FDT_ADDR"
            ;;
        -i:*)   INITRD=( ${1#-i:} )
		        echo "FIT initrd/initramfs with $INITRD"
            ;;
        -r:*)   ROOTFS=( ${1#-r:} )
		        echo "FIT rootfs with $ROOTFS"
            ;;
		* ) echo "Invalid option passed to '$0' (options:$*)"
		exit 1;;
    esac
    shift
done

CURR_DIR=$(pwd)
IMAGE_DIR="$CURR_DIR/build/arch/arm64/boot"
OUTPUT_DIR="$CURR_DIR/build_out"
BUILD_DIR="$CURR_DIR/build"

if [ "$MakeMode" == "all" ]; then
#clean build
rm -rf  build build_out
mkdir -p build_out

make dubhe1000_tiny_defconfig ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build
make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build
fi

if [ "$MakeMode" == "only" ]; then

	if [[ -d build ]] && [[ -d build_out ]]; then
		echo "alreay have build and build_out folder"
	else
		rm -rf  build build_out
		mkdir -p build_out
		make dubhe1000_tiny_defconfig ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build
	fi

make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- -C ./ O=build
fi

cd $IMAGE_DIR
if [[ -d out_hex ]] && [[ -d out_split ]]; then
	rm -rf  out_hex out_split
fi
mkdir -p out_hex out_split
#echo "#EMU Load Memory Image Script" > out_hex/emu_memory.script
split -b 64k Image out_split/Image-

if [[ -f emu_memory.script ]]; then
	rm -f  emu_memory.script
fi

cd out_split
for p in $(ls Image-*) ; do
#echo "$p"
	hexdump -v -e/'4 "%08x\n"' $p > ../out_hex/$p.hex
	echo "memory -load %readmemh {U_SOC_TOP.U_CCI_SEC_SUB.U_IRAM.U_RAM_INST.\DEPTH_RAM_INST[$kernel_blcok].WIDTH_RAM_INST[0].u_ram_sp_16384x32b1s011_iram_emu_wrp2 .u_eda_ram_sp.mem} -file out_hex/$p.hex -start 0x0" >> ../emu_memory.script
    kernel_blcok=$((kernel_blcok + 1))
done
cd ..


# build dtb emu memory command
hexdump -v -e/'4 "%08x\n"' dts/clourney/$kernel_dtb > out_hex/$kernel_dtb.hex
echo "memory -load %readmemh {U_SOC_TOP.U_CCI_SEC_SUB.U_IRAM.U_RAM_INST.\DEPTH_RAM_INST[$dtb_blcok].WIDTH_RAM_INST[0].u_ram_sp_16384x32b1s011_iram_emu_wrp2 .u_eda_ram_sp.mem} -file out_hex/$kernel_dtb.hex -start 0x0" >> ./emu_memory.script

#tar.gz out_hex
cd $IMAGE_DIR
tar czf out_hex.tar.gz out_hex
cp out_hex.tar.gz emu_memory.script Image Image.gz dts/clourney/$kernel_dtb $OUTPUT_DIR
cp $BUILD_DIR/System.map $BUILD_DIR/vmlinux $OUTPUT_DIR

echo "build kernel image successful"
#aarch64-none-linux-gnu-objdump -S -d $OUTPUT_DIR/vmlinux > $OUTPUT_DIR/vmlinux.s
#echo "aarch64-none-linux-gnu-objdump -S -d $OUTPUT_DIR/vmlinux > $OUTPUT_DIR/vmlinux.s done"

cd $CURR_DIR
cp mkits.sh $OUTPUT_DIR
cp $INITRD $OUTPUT_DIR
cd $OUTPUT_DIR

INITRD_MKITS_PARA=""
ROOTFS_MKITS_PARA=""

if [ "$INITRD" != "" ]; then
   INITRD_MKITS_PARA="-i $INITRD -Z $InitrdCompress"
fi

if [ "$ROOTFS" != "" ]; then
   ROOTFS_MKITS_PARA="-r $ROOTFS"
fi

if [ "$KernelCompress" == "none" ]; then
   ./mkits.sh -A arm64 -D dubhe1000 -a $ENTRY_ADDR -e $ENTRY_ADDR -v 5.10.111 -k Image -C $KernelCompress -d $kernel_dtb -o dubhe1000-emu-kernel.its -c "standard" -t $FDT_ADDR $INITRD_MKITS_PARA $ROOTFS_MKITS_PARA
fi

if [ "$KernelCompress" != "none" ]; then
   ./mkits.sh -A arm64 -D dubhe1000 -a $ENTRY_ADDR -e $ENTRY_ADDR -v 5.10.111 -k Image.gz -C $KernelCompress -d $kernel_dtb -o dubhe1000-emu-kernel.its -c "standard" -t $FDT_ADDR $INITRD_MKITS_PARA $ROOTFS_MKITS_PARA
fi

mkimage -f dubhe1000-emu-kernel.its dubhe1000-emu-kernel.bin

if [[ -d fit_hex ]] && [[ -d fix_split ]]; then
	rm -rf fit_hex fix_split
fi
mkdir -p fit_hex fix_split

split -b 64k dubhe1000-emu-kernel.bin fix_split/dubhe1000-emu-kernel-

if [[ -f fit_image_emu_memory.script ]] || [[ -f fit_hex.tar.gz ]]; then
	rm -f  fit_image_emu_memory.script fit_hex.tar.gz
fi

cd  fix_split
for p in $(ls dubhe1000-emu-kernel-*) ; do
#echo "$p"
	hexdump -v -e/'4 "%08x\n"' $p > ../fit_hex/$p.hex
	echo "memory -load %readmemh {U_SOC_TOP.U_CCI_SEC_SUB.U_IRAM.U_RAM_INST.\DEPTH_RAM_INST[$FitImage_blcok].WIDTH_RAM_INST[0].u_ram_sp_16384x32b1s011_iram_emu_wrp2 .u_eda_ram_sp.mem} -file fit_hex/$p.hex -start 0x0" >> ../fit_image_emu_memory.script
    FitImage_blcok=$((FitImage_blcok + 1))
done

cd $OUTPUT_DIR
tar czf fit_hex.tar.gz fit_hex
rm -rf fit_hex fix_split
echo "build kernel FIT Image successful"

