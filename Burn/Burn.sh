#!/bin/sh
#########################################################
#   description: Burn test script
#   usage: Burn.sh
#   version:2014/5/22 by Ryan
########################################################

# CPU stress test
nohup dhrystone 10000000000000000000 > cpu_stress.log 2>&1 &

# Memory stress test
MEMSIZE=`cat /proc/meminfo | grep "MemTotal:" | awk {'print $2'}`
UNIT=`cat /proc/meminfo | grep "MemTotal:" | awk {'print $3'} | cut -c 1`
MEMSIZE=`expr $MEMSIZE \* 8 / 10`
echo $MEMSIZE$UNIT
./memtester $MEMSIZE$UNIT > mem_stress.log 2>&1 &
