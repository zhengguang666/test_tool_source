#!/system/bin/sh
#########################################################
#   description: Burn test script
#   usage: Burn.sh
#   version:2014/5/22 by Ryan
########################################################

while true
do
  for name in `ls /mnt/media_rw`
    do
      echo "copy file from EMMC to External-Storage's " $name
      cp ./test.txt /mnt/media_rw/$name
    done
./loopback_uart232_S
./loopback_uart232_USB
done




