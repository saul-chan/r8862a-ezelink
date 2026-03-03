CC		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip

INPUT0 += ../npe_main.c
INPUT0 += ../npe_edma.c
INPUT0 += ../npe_switch.c 
#SWITCH_CMD
INPUT1 += switch_cmd.c
INPUT1 += switch_vlan.c switch_generic.c switch_l2.c switch_l3.c
INPUT1 += switch_tunnel.c switch_learning.c switch_mac_range.c
INPUT1 += switch_acl.c switch_nat.c

OUTPUT = npecmd

all:
	$(CC) -static -I ../ -I ./ -o $(OUTPUT) $(INPUT0) $(INPUT1)
	$(STRIP) $(OUTPUT)
