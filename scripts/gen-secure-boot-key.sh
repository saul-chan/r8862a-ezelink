#!/bin/bash

if [ $# != 1 ]; then
	echo "Usage: $0 <key_file_name>"
	exit 1
fi

key_file_name=$1
rm -f $key_file_name.key $key_file_name.crt
mkdir -p $(dirname $key_file_name)

echo "generate $key_file_name.key"
openssl genpkey -algorithm RSA -out $key_file_name.key  -pkeyopt rsa_keygen_bits:2048 -pkeyopt rsa_keygen_pubexp:65537
echo "generate $key_file_name.crt"
openssl req -batch -new -x509 -key $key_file_name.key -out $key_file_name.crt
