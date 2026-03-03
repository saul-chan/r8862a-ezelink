#!/bin/bash

INTERFACE_NAME=""
IFACE_PROT_NO=""
IFACE_IO_NO=""

pad_pdu_write() {
	settingAddr=$(printf "0x%08x" $1)
	settingVal=$(printf "0x%08x" $2)
	devmem $settingAddr 32 $settingVal
	#echo "devmem $settingAddr 32 $settingVal"
}

pad_pdu_read() {
	settingAddr=$(printf "0x%08x" $1)
	devmem $settingAddr 32
	#echo "devmem $settingAddr 32"
}

BOOT_CFGx_0_PadArray="0x90420000 0x90420004 0x90420008 0x9042000C 0x90420010 0x90420014"
BOOT_CFGx_1_PadArray="0x90430000 0x90430004 0x90430008 0x9043000C 0x90430010 0x90430014"

FEM_CTRL0_PadArray="0x90410000 0x90410004 0x90410008"
FEM_CTRL1_PadArray="0x9041000C 0x90410010 0x90410014"
FEM_CTRL2_PadArray="0x90420000 0x90420004 0x90420008"
FEM_CTRL3_PadArray="0x9042000C 0x90420010 0x90420014"

USB_JTAG_PadArray="0x90420000 0x90420004 0x90420008 0x9042000C 0x90420010"
SDS0_JTAG_PadArray="0x904200030 0x90420034 0x90420038 0x9042003C 0x90420040"
SDS1_JTAG_PadArray="0x904200048 0x9042004C 0x90420050 0x90420054 0x90420058"
MAIN_JTAG_PadArray="0x90420060 0x90420064 0x90420068 0x9042006C 0x90420070"
PCIE_JTAG_PadArray="0x904200078 0x9042007C 0x90420080 0x90420084 0x90420088"

SFLASH_PadArray="0x90420018 0x9042001C 0x90420020 0x90420024 0x90420028 0x9042002C"
NAND_PadArray="0x90420018 0x9042001C 0x90420020 0x90420024 0x90420028 0x9042002C 0x90420030 0x90420034 0x90420038 0x9042003C 0x90420040 0x90420044 0x90420048 0x9042004C 0x90420050"
EMMC_PadArray="0x90420030 0x90420034 0x90420038 0x9042003C 0x90420040 0x90420044 0x90420048 0x9042004C 0x90420050 0x90420054 0x90420058"

RGMII0_PadArray="0x90460008 0x9046000C 0x90460010 0x90460014 0x90460018 0x9046001C 0x90460020 0x90460024 0x90460028 0x9046002C 0x90460030 0x90460034"
RGMII1_PadArray="0x90410018 0x9041001C 0x90410020 0x90410024 0x90410028 0x9041002C 0x90410030 0x90410034 0x90410038 0x9041003C 0x90410040 0x90410044"
RGMII2_PadArray="0x90420030 0x90420034 0x90420038 0x9042003C 0x90420040 0x90420044 0x90420048 0x9042004C 0x90420050 0x90420054 0x90420058 0x9042005C"

UART0_0_PadArray="0x90460018 0x9046001C 0x90460020 0x90460024"
UART0_1_PadArray="0x90460008 0x9046000C 0x90460010 0x90460014"
UART0_2_PadArray="0x90410018 0x9041001C 0x90410020 0x90410024"
UART1_0_PadArray="0x90420060 0x90420064 0x90420068 0x9042006C"
UART1_1_PadArray="0x90420030 0x90420034 0x90420038 0x9042003C"
UART1_2_PadArray="0x90420074 0x90420078 0x9042007C 0x90420080"
UART2_0_PadArray="0x9042005C 0x90420084 0x90420088 0x9042008C"
UART2_1_PadArray="0x90420048 0x9042004C 0x90420050 0x90420054"
UART2_2_PadArray="0x90410028 0x9041002C 0x90410030 0x90410034"

SPI0_0_PadArray="0x90460018 0x9046001C 0x90460020 0x90460024"
SPI0_1_PadArray="0x90460008 0x9046000C 0x90460010 0x90460014"
SPI0_2_PadArray="0x90410018 0x9041001C 0x90410020 0x90410024"
SPI1_0_PadArray="0x90420060 0x90420064 0x90420068 0x9042006C"
SPI1_1_PadArray="0x90420030 0x90420034 0x90420038 0x9042003C"
SPI1_2_PadArray="0x90420074 0x90420078 0x9042007C 0x90420080"
SPI2_0_PadArray="0x9042005C 0x90420084 0x90420088 0x9042008C"
SPI2_1_PadArray="0x90420048 0x9042004C 0x90420050 0x90420054"
SPI2_2_PadArray="0x90410028 0x9041002C 0x90410030 0x90410034"

I2C0_0_PadArray="0x9042007C 0x90420080"
I2C0_1_PadArray="0x90460018 0x9046001C"
I2C0_2_PadArray="0x90460008 0x9046000C"
I2C1_0_PadArray="0x90410050 0x90410054"
I2C1_1_PadArray="0x90420030 0x90420034"
I2C2_2_PadArray="0x90420038 0x9042003C"
I2C2_0_PadArray="0x90460010 0x90460014"
I2C2_1_PadArray="0x90420088 0x9042008C"
I2C2_2_PadArray="0x90420038 0x9042003C"

PWMx_0_PadArray="0x90430000 0x90430004 0x90430008 0x9043000C 0x90430010 0x90430014"
PWMx_1_PadArray="0x90420048 0x9042004C 0x90420050 0x90420054"
PWMX_2_PadArray="0x90410038 0x9041003C 0x90410040 0x90410044"

I2S0_PadArray="0x90460018 0x9046001C 0x90460020 0x90460024"
I2S1_PadArray="0x90410038 0x9041003C 0x90410040 0x90410044"
I2S2_PadArray="0x90420074 0x90420078 0x9042007C 0x90420080"

SAC_5G_PadArray="0x90420074 0x90420078 0x9042007C 0x90420080"
SAC_2G_PadArray="0x90460008 0x9046000C 0x90460010 0x90460014"

MII0_PadArray="0x90410048 0x9041004C"
MII1_PadArray="0x90410050 0x90410054"
MII1_PadArray="0x90420074 0x90420078"

QSPI_PadArray="0x90420074 0x90420078 0x9042007C 0x90420080 0x90420084 0x90420088"

GPIO_PadArray="0x90420030 0x90420034 0x90420038 0x9042003C
	       0x90420040 0x90420044 0x90420048 0x9042004C
	       0x90420050 0x90420054 0x90420058 0x9042005C
	       0x90420060 0x90420064 0x90420068 0x9042006C
	       0x90420070 0x90420074 0x90420078 0x9042007C
	       0x90420080 0x90420084 0x90420088 0x9042008C
	       0x90410018 0x9041001C 0x90410020 0x90410024
	       0x90410028 0x9041002C 0x90410030 0x90410034
	       0x90410038 0x9041003C 0x90410040 0x90410044
	       0x90410048 0x9041004C 0x90410050 0x90410054
	       0x90410058 0x9041005C 0x90410014 0x90460008
	       0x9046000C 0x90460010 0x90460014 0x90460018
	       0x9046001C 0x90460020 0x90460024 0x90460028
	       0x9046002C 0x90460030 0x90460034 0x90430000
	       0x90430004 0x90430008 0x9043000C 0x90430010
	       0x90430014 0x90420008 0x90420014 0x90410008"

GE_PHY_REF_CLK_PadArray="0x90410050 0x90410054 0x90410008 0x90410014 0x90410058 0x9041005C"

io_default_level=8
pad_func_bit=12

io_select_setting() {
	IO_AND_VALUE=$(printf "0x%08X" $1)
	IO_OR_VALUE=$(printf "0x%08X" $2)
	IO_SEL_REG="0x90000094"
	#IO_SEL_REGVAL=$(devmem $IO_SEL_REG 32)
	IO_SEL_REGVAL=0
	IO_SET_VALUE=$((IO_SEL_REGVAL & IO_AND_VALUE))
	IO_SEL_REGVAL=$((IO_SET_VALUE | IO_OR_VALUE))
	pad_pdu_write $IO_SEL_REG $IO_SEL_REGVAL
}

uart_pad_io_setting() {
	UART_IO_SEL_MASK=3
	UART0_IO_SEL_BIT=10
	UART1_IO_SEL_BIT=8
	UART2_IO_SEL_BIT=6
	UART_IFACE="UART""$1-$2"
	case $UART_IFACE in
		"UART0-0" )
				for padReg in $UART0_0_PadArray;
				do
					padRegVal=$(((0 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART0_IO_SEL_BIT))) $((0 << UART0_IO_SEL_BIT))
				;;
		"UART0-1" )
				for padReg in $UART0_1_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART0_IO_SEL_BIT))) $((1 << UART0_IO_SEL_BIT))
				;;
		"UART0-2" )
				for padReg in $UART0_2_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART0_IO_SEL_BIT))) $((2 << UART0_IO_SEL_BIT))
				;;
		"UART1-0" )
				for padReg in $UART1_0_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART1_IO_SEL_BIT))) $((0 << UART1_IO_SEL_BIT))
				;;
		"UART1-1" )
				for padReg in $UART1_1_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART1_IO_SEL_BIT))) $((1 << UART1_IO_SEL_BIT))
				;;
		"UART1-2" )
				for padReg in $UART1_2_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART1_IO_SEL_BIT))) $((2 << UART1_IO_SEL_BIT))
				;;
		"UART2-0" )
				for padReg in $UART2_0_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART2_IO_SEL_BIT))) $((0 << UART2_IO_SEL_BIT))
				;;
		"UART2-1" )
				for padReg in $UART2_1_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART2_IO_SEL_BIT))) $((1 << UART2_IO_SEL_BIT))
				;;
		"UART2-2" )
				for padReg in $UART2_2_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(UART_IO_SEL_MASK << UART2_IO_SEL_BIT))) $((2 << UART2_IO_SEL_BIT))
				;;
			* )	echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

i2c_pad_io_setting() {
	I2C_IO_SEL_MASK=3
	I2C0_IO_SEL_BIT=4
	I2C1_IO_SEL_BIT=2
	I2C2_IO_SEL_BIT=0
	I2C_IFACE="I2C""$1-$2"
	case $I2C_IFACE in
		"I2C0-0" )
				for padReg in $I2C0_0_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C0_IO_SEL_BIT))) $((0 << I2C0_IO_SEL_BIT))
				;;
		"I2C0-1" )
				for padReg in $I2C0_1_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C0_IO_SEL_BIT))) $((1 << I2C0_IO_SEL_BIT))
				;;
		"I2C0-2" )
				for padReg in $I2C0_2_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C0_IO_SEL_BIT))) $((2 << I2C0_IO_SEL_BIT))
				;;
		"I2C1-0" )
				for padReg in $I2C1_0_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C1_IO_SEL_BIT))) $((0 << I2C1_IO_SEL_BIT))
				;;
		"I2C1-1" )
				for padReg in $I2C1_1_PadArray;
				do
					padRegVal=$(((5 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C1_IO_SEL_BIT))) $((1 << I2C1_IO_SEL_BIT))
				;;
		"I2C1-2" )
				for padReg in $I2C1_2_PadArray;
				do
					padRegVal=$(((5 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C1_IO_SEL_BIT))) $((2 << I2C1_IO_SEL_BIT))
				;;
		"I2C2-0" )
				for padReg in $I2C2_0_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C2_IO_SEL_BIT))) $((0 << I2C2_IO_SEL_BIT))
				;;
		"I2C2-1" )
				for padReg in $I2C2_1_PadArray;
				do
					padRegVal=$(((5 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C2_IO_SEL_BIT))) $((1 << I2C2_IO_SEL_BIT))
				;;
		"I2C2-2" )
				for padReg in $I2C2_2_PadArray;
				do
					padRegVal=$(((5 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2C_IO_SEL_MASK << I2C2_IO_SEL_BIT))) $((2 << I2C2_IO_SEL_BIT))
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

spi_pad_io_setting() {
	SPI_IO_SEL_MASK=3
	SPI0_IO_SEL_BIT=16
	SPI1_IO_SEL_BIT=14
	SPI2_IO_SEL_BIT=12
	SPI_IFACE="SPI""$1-$2"
	case $SPI_IFACE in
		"SPI0-0" )
				for padReg in $SPI0_0_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI0_IO_SEL_BIT))) $((0 << SPI0_IO_SEL_BIT))
				;;
		"SPI0-1" )
				for padReg in $SPI0_1_PadArray;
				do
					padRegVal=$(((0 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI0_IO_SEL_BIT))) $((1 << SPI0_IO_SEL_BIT))
				;;
		"SPI0-2" )
				for padReg in $SPI0_2_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI0_IO_SEL_BIT))) $((2 << SPI0_IO_SEL_BIT))
				;;
		"SPI1-0" )
				for padReg in $SPI1_0_PadArray;
				do
					padRegVal=$(((4 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI1_IO_SEL_BIT))) $((0 << SPI1_IO_SEL_BIT))
				;;
		"SPI1-1" )
				for padReg in $SPI1_1_PadArray;
				do
					padRegVal=$(((4 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI1_IO_SEL_BIT))) $((1 << SPI1_IO_SEL_BIT))
				;;
		"SPI1-2" )
				for padReg in $SPI1_2_PadArray;
				do
					padRegVal=$(((4 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI1_IO_SEL_BIT))) $((2 << SPI1_IO_SEL_BIT))
				;;
		"SPI2-0" )
				for padReg in $SPI2_0_PadArray;
				do
					padRegVal=$(((4 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI2_IO_SEL_BIT))) $((0 << SPI2_IO_SEL_BIT))
				;;
		"SPI2-1" )
				for padReg in $SPI2_1_PadArray;
				do
					padRegVal=$(((4 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI2_IO_SEL_BIT))) $((1 << SPI2_IO_SEL_BIT))
				;;
		"SPI2-2" )
				for padReg in $SPI2_2_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(SPI_IO_SEL_MASK << SPI2_IO_SEL_BIT))) $((2 << SPI2_IO_SEL_BIT))
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

i2s_pad_io_setting() {
	I2S_IO_SEL_MASK=3
	I2S_IO_SEL_BIT=18
	I2S_IFACE="I2S""$1"
	case $I2S_IFACE in
		"I2S0" )
				for padReg in $I2S0_PadArray;
				do
					padRegVal=$(((3 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2S_IO_SEL_MASK << I2S_IO_SEL_BIT))) $((0 << I2S_IO_SEL_BIT))
				;;
		"I2S1" )
				for padReg in $I2S1_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2S_IO_SEL_MASK << I2S_IO_SEL_BIT))) $((1 << I2S_IO_SEL_BIT))
				;;
		"I2S2" )
				for padReg in $I2S2_PadArray;
				do
					padRegVal=$(((5 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				io_select_setting $((~(I2S_IO_SEL_MASK << I2S_IO_SEL_BIT))) $((2 << I2S_IO_SEL_BIT))
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
	}

rgmii_pad_io_setting() {
	RGMII_IFACE="RGMII""$1"
	case $RGMII_IFACE in
		"RGMII0" )
				for padReg in $RGMII0_PadArray;
				do
					padRegVal=$(((4 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"RGMII1" )
				for padReg in $RGMII1_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"RGMII2" )
				for padReg in $RGMII2_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

mii_pad_io_setting() {
	MII_IFACE="MII""$1"
	case $MII_IFACE in
		"MII0" )
				for padReg in $MII0_PadArray;
				do
					padRegVal=$(((4 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"MII1" )
				for padReg in $MII1_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"MII2" )
				for padReg in $MII2_PadArray;
				do
					padRegVal=$(((1 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

pwm_pad_io_setting() {
	PWM_IO_SEL_MASK=3
	PWM0_IO_SEL_BIT=26
	PWM1_IO_SEL_BIT=24
	PWM2_IO_SEL_BIT=22
	PWM3_IO_SEL_BIT=20
	PWM_IFACE="PWMx""_$2"
	pwm_port_no=0
	case $PWM_IFACE in
		"PWMx_0" )
				for padReg in $PWMx_0_PadArray;
				do
					if [ $pwm_port_no -eq $1 ]; then
						padRegVal=$(((1 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
						case $pwm_port_no in
							0 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM0_IO_SEL_BIT))) $((0 << PWM0_IO_SEL_BIT))
								;;
							1 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM1_IO_SEL_BIT))) $((0 << PWM1_IO_SEL_BIT))
								;;
							2 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM2_IO_SEL_BIT))) $((0 << PWM2_IO_SEL_BIT))
								;;
							3 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM3_IO_SEL_BIT))) $((0 << PWM3_IO_SEL_BIT))
								;;
						esac
					fi
					pwm_port_no=$(($pwm_port_no + 1))
				done
				;;
		"PWMx_1" )
				for padReg in $PWMx_1_PadArray;
				do
					if [ $pwm_port_no -eq $1 ]; then
						padRegVal=$(((5 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
						case $pwm_port_no in
							0 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM0_IO_SEL_BIT))) $((1 << PWM0_IO_SEL_BIT))
								;;
							1 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM1_IO_SEL_BIT))) $((1 << PWM1_IO_SEL_BIT))
								;;
							2 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM2_IO_SEL_BIT))) $((1 << PWM2_IO_SEL_BIT))
								;;
							3 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM3_IO_SEL_BIT))) $((1 << PWM3_IO_SEL_BIT))
								;;
						esac
					fi
					pwm_port_no=$(($pwm_port_no + 1))
				done
				;;
		"PWMx_2" )
				for padReg in $PWMx_2_PadArray;
				do
					if [ $pwm_port_no -eq $1 ]; then
						padRegVal=$(((3 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
						case $pwm_port_no in
							0 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM0_IO_SEL_BIT))) $((2 << PWM0_IO_SEL_BIT))
								;;
							1 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM1_IO_SEL_BIT))) $((2 << PWM1_IO_SEL_BIT))
								;;
							2 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM2_IO_SEL_BIT))) $((2 << PWM2_IO_SEL_BIT))
								;;
							3 )
								io_select_setting $((~(PWM_IO_SEL_MASK << PWM3_IO_SEL_BIT))) $((2 << PWM3_IO_SEL_BIT))
								;;
						esac
					fi
					pwm_port_no=$(($pwm_port_no + 1))
				done
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

bootcfg_pad_io_setting() {
	BOOT_CFG_IFACE="BOOT_CFGx""_$2"
	bootcfg_port_no=0
	case $BOOT_CFG_IFACE in
		"BOOT_CFGx_0" )
				for padReg in $BOOT_CFGx_0_PadArray;
				do
					if [ $bootcfg_port_no -eq $1 ]; then
						padRegVal=$(((0 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
					fi
					bootcfg_port_no=$(($bootcfg_port_no + 1))
				done
				;;
		"BOOT_CFGx_1" )
				for padReg in $BOOT_CFGx_1_PadArray;
				do
					if [ $bootcfg_port_no -eq $1 ]; then
						padRegVal=$(((0 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
					fi
					bootcfg_port_no=$(($bootcfg_port_no + 1))
				done
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

femctrl_pad_io_setting() {
	FEM_CTRL_IFACE="FEM_CTRL""$1"
	femctrl_port_no=0
	case $FEM_CTRL_IFACE in
		"FEM_CTRL0" )
				for padReg in $FEM_CTRL0_PadArray;
				do
					if [ $femctrl_port_no -eq $2 ]; then
						padRegVal=$(((1 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
					fi
					femctrl_port_no=$(($femctrl_port_no + 1))
				done
				;;
		"FEM_CTRL1" )
				for padReg in $FEM_CTRL1_PadArray;
				do
					if [ $femctrl_port_no -eq $2 ]; then
						padRegVal=$(((1 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
					fi
					femctrl_port_no=$(($femctrl_port_no + 1))
				done
				;;
		"FEM_CTRL2" )
				for padReg in $FEM_CTRL2_PadArray;
				do
					if [ $femctrl_port_no -eq $2 ]; then
						padRegVal=$(((1 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
					fi
					femctrl_port_no=$(($femctrl_port_no + 1))
				done
				;;
		"FEM_CTRL3" )
				for padReg in $FEM_CTRL3_PadArray;
				do
					if [ $femctrl_port_no -eq $2 ]; then
						padRegVal=$(((1 << pad_func_bit) | io_default_level))
						pad_pdu_write $padReg $padRegVal
					fi
					femctrl_port_no=$(($femctrl_port_no + 1))
				done
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

ge_phy_ref_clk_pad_io_setting() {
	ge_phy_ref_clk_port_no=0
	for padReg in $GE_PHY_REF_CLK_PadArray;
	do
		if [ $ge_phy_ref_clk_port_no -eq $1 ]; then
			padRegVal=$(((3 << pad_func_bit) | io_default_level))
			pad_pdu_write $padReg $padRegVal
		fi
		ge_phy_ref_clk_port_no=$(($ge_phy_ref_clk_port_no + 1))
	done
}

gpio_pad_io_setting() {
	gpio_no=0
	for padReg in $GPIO_PadArray;
	do
		if [ $gpio_no -eq $1 ]; then
			padRegVal=$(((6 << pad_func_bit) | io_default_level))
			pad_pdu_write $padReg $padRegVal
		fi
		gpio_no=$(($gpio_no + 1))
	done
}

special_pad_io_setting() {
	SPECIAL_IFACE_NAME="$1"
	case $SPECIAL_IFACE_NAME in
		"USB_JTAG" )
				for padReg in $USB_JTAG_PadArray;
				do
					padRegVal=$(((7 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"SDS0_JTAG" )
				for padReg in $SDS0_JTAG_PadArray;
				do
					padRegVal=$(((7 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"SDS1_JTAG" )
				for padReg in $SDS1_JTAG_PadArray;
				do
					padRegVal=$(((7 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"MAIN_JTAG" )
				for padReg in $MAIN_JTAG_PadArray;
				do
					padRegVal=$(((0 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"PCIE_JTAG" )
				for padReg in $PCIE_JTAG_PadArray;
				do
					padRegVal=$(((7 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"SFLASH" )
				for padReg in $SFLASH_PadArray;
				do
					padRegVal=$(((0 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"NAND" )
				for padReg in $NAND_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"EMMC" )
				for padReg in $EMMC_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"SAC_5G" )
				for padReg in $SAC_5G_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"SAC_2G" )
				for padReg in $SAC_2G_PadArray;
				do
					padRegVal=$(((2 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		"QSPI" )
				for padReg in $QSPI_PadArray;
				do
					padRegVal=$(((0 << pad_func_bit) | io_default_level))
					pad_pdu_write $padReg $padRegVal
				done
				;;
		* )		echo "Invalid option passed to '$0' (options:$*)"
				exit
				;;
	esac
}

pad_io_setting() {
	PAD_IFACE_NAME="$1"
	case $PAD_IFACE_NAME in
		"UART" )
			uart_pad_io_setting $2 $3
			;;
		"SPI" )
			spi_pad_io_setting $2 $3
			;;
		"I2C" )
			i2c_pad_io_setting $2 $3
			;;
		"I2S" )
			i2s_pad_io_setting $2
			;;
		"RGMII" )
			rgmii_pad_io_setting $2
			;;
		"MII" )
			mii_pad_io_setting $2
			;;
		"PWM" )
			pwm_pad_io_setting $2 $3
			;;
		"BOOT_CFG" )
			bootcfg_pad_io_setting $2 $3
			;;
		"FEM_CTRL" )
			femctrl_pad_io_setting $2 $3
			;;
		"GE_PHY_REF_CLK" )
			ge_phy_ref_clk_pad_io_setting $2
			;;
		"GPIO" )
			gpio_pad_io_setting $2
			;;
		* )
			special_pad_io_setting $PAD_IFACE_NAME
			;;
	esac
}

usage() {
	printf "Usage: %s" "$(basename "$0")"

	printf "\n\t-i ==> SoC Low speed interface name"
	printf "\n\t       'UART' 'SPI' 'I2C' 'I2S' 'PWM' 'GPIO' 'RGMII' 'MII' 'GE_PHY_REF_CLK' 'BOOT_CFG'"
	printf "\n\t       'FEM_CTRL' 'EMMC' 'NAND' 'SFLASH' 'SAC_5G' 'SAC_2G' 'QSPI'"
	printf "\n\t       'MAIN_JTAG' 'USB_JTAG' 'PCIE_JTAG' 'SDS0_JTAG' 'SDS0_JTAG'"
	printf "\n\t-p ==> Low speed interface's Port Num."
	printf "\n\t-n ==> Low speed interface's Pad no."
	printf "\n\r"
	exit 1
}

while getopts "i:n:p:" OPTION
do
	case $OPTION in
		i )   IFACE_NAME="$OPTARG"
			;;
		n )   IFACE_IO_NO="$OPTARG"
			;;
		p )   IFACE_PORT_NO="$OPTARG"
			;;
		* ) echo "Invalid option passed to '$0' (options:$*)"
			usage;;
	esac
done

#echo "pad_io_setting $IFACE_NAME $IFACE_PORT_NO $IFACE_IO_NO"
pad_io_setting $IFACE_NAME $IFACE_PORT_NO $IFACE_IO_NO
