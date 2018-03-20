#!/bin/sh
#########################################################
#   description: Burn test script
#   usage: Burn.sh
#   version:2014/5/22 by Ryan
########################################################

#busybox ifconfig eth0 192.168.1.100 up
#busybox ifconfig eth1 192.168.2.100 up

./stress --cpu 4 --io 4 --vm 1 --vm-bytes 64M &

# CPU stress test
#busybox nohup ./dhrystone 10000000000000000000 > cpu_stress.log 2>&1 &

#Memory stress test
#MEMSIZE=`cat /proc/meminfo | grep "MemTotal:" | busybox awk {'print $2'}` 
#UNIT=`cat /proc/meminfo | grep "MemTotal:" | busybox awk {'print $3'} | busybox cut -c 1`
#MEMSIZE=`busybox expr $MEMSIZE \* 8 / 10`
#MEMSIZE=`busybox expr $MEMSIZE \* 8 / 10 - 200000`

MEMSIZE=150000k
./memtester $MEMSIZE$UNIT > mem_stress.log 2>&1 &

./SATA-USB.sh > /dev/null &

echo "cp2104 usb->serial test"
./uart &
	
ifconfig eth0 192.168.1.1
ifconfig eth1 192.168.2.1

ping 192.168.1.100 -s 1500 -i 1 &
ping 192.168.2.100 -s 1500 -i 1 &

while [ 1 = 1 ] ;do
	echo "stm32 485 test"
	echo "@" > /dev/ttymxc0
	sleep 1
	echo "#" > /dev/ttymxc0
	sleep 1

	echo stm32 io test ,io up
	echo "$" > /dev/ttymxc0
	sleep 1
	echo stm32 io test ,io down
	echo "%" > /dev/ttymxc0
	sleep 1

	echo "leds up"
	echo "^" > /dev/ttymxc0
	sleep 1
	echo "led off"
	echo "&" > /dev/ttymxc0
	sleep 1

	echo "can test"
	echo "(" > /dev/ttymxc0
	sleep 1

	echo "spi test"
	echo "~" > /dev/ttymxc0
	sleep 3
done



