#!/bin/bash

FILENAME=$1
rm -rf ${FILENAME}-*
split -d -b 64k ${FILENAME} ${FILENAME}-bin-
for COUNT in $(seq 0 99); do
	if [ "$COUNT" -gt "9" ]
	then
		BINNAME=${FILENAME}"-bin-"${COUNT}
		HEXNAME=${FILENAME}"-hex-"${COUNT}
	else
		BINNAME=${FILENAME}"-bin-0"${COUNT}
		HEXNAME=${FILENAME}"-hex-0"${COUNT}
	fi
	if [ -f ${BINNAME} ]
	then
		hexdump -v -e/'4 "%08x\n"' ${BINNAME} > ${HEXNAME}
		COUNT=`expr $COUNT + 1`
	else
		rm -rf ${FILENAME}-bin-*
		echo "Total $COUNT files generated."
		exit
	fi
done
