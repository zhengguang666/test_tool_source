#!/system/bin/sh
#########################################################
#   description: Burn test script
#   usage: Burn.sh
#   version:2014/5/22 by Ryan
########################################################

busybox ifconfig eth0 192.168.1.100 up

./stress --cpu 8 --io 4 --vm 2 --vm-bytes 128M &

# CPU stress test
busybox nohup ./dhrystone 10000000000000000000 > cpu_stress.log 2>&1 &

# Memory stress test
MEMSIZE=`cat /proc/meminfo | grep "MemTotal:" | busybox awk {'print $2'}`
UNIT=`cat /proc/meminfo | grep "MemTotal:" | busybox awk {'print $3'} | busybox cut -c 1`
#MEMSIZE=`busybox expr $MEMSIZE \* 8 / 10`
MEMSIZE=`busybox expr $MEMSIZE \* 8 / 10 - 100000`
echo $MEMSIZE$UNIT
./memtester $MEMSIZE$UNIT > mem_stress.log 2>&1 &

./File-RW.sh &

./gpio_test.sh &
