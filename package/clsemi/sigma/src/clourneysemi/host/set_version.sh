#!/bin/bash
# Copyright (c) 2023 Clourney Semiconductor Co.,Ltd.

change_num=`p4 changes -m 1 ...#have 2>/dev/null | cut -d' ' -f 2-2`
if [ "$change_num" = "" ]; then
	echo "Perforce change number not found"
	change_num='unknown'
fi

version=`cat version.txt`

#update package info
rm -f debian/changelog
dch --create \
	--package "sigma-ca-proxy" \
	--vendor "Clourney Semiconductor Co.,Ltd" \
	-v "${version}build${change_num}" \
	"Development build ${version}build${change_num}" \
	-M
