#!/bin/bash

previous_ver=$1

if [ ! -d $previous_ver ];then
	echo "$previous_ver not exist"
	exit
fi

for table in *.bin;do

	pre_ver_md5=$(dd if=$previous_ver/$table bs=1 skip=5|md5sum -b| awk '{print $1}')
	cur_ver_md5=$(dd if=$table bs=1 skip=5|md5sum -b| awk '{print $1}')
	if [ "$pre_ver_md5" == "$cur_ver_md5" ];then
		echo "$table == $previous_ver/$table, replace with link"
		echo "md5: $pre_ver_md5"
		ln -sf $previous_ver/$table  $table 
	else 
		echo "md5: $pre_ver_md5 $cur_ver_md5"
		echo ">>>>>>>>>> $table is new! <<<<<<<<<<"
	fi

done
