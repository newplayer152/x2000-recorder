#!/bin/bash
cd /mnt/sdcard
for i in *.txt; do
  tftp -l "$i" -p 192.168.1.228  #下载ip位置
done

