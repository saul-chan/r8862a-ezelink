#!/bin/sh

#/****************************************************************************
# Copyright (c) 2014 Wi-Fi Alliance
# 
# Permission to use, copy, modify, and/or distribute this software for any 
# purpose with or without fee is hereby granted, provided that the above 
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY 
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
# USE OR PERFORMANCE OF THIS SOFTWARE.
#*****************************************************************************/

# Install the SigmaWTG service to system startup

sh $PWD/SigmaWTGService.sh stop
cp -f ../scripts/* /usr/local/sbin/
cp  SigmaWTG ../dut/wfa_dut ../ca/wfa_ca /usr/bin
cp  SigmaWTG.conf /etc
mkdir /SIGMA_WTGv2
rm -f /etc/rc.local
ln -s $PWD/SigmaWTGService.sh /etc/rc.local
sh $PWD/SigmaWTGService.sh start

