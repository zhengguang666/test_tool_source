#!/bin/bash

# A wrap for uc to test serial port
#jia.sui@advantech.com.cn
#2015-05-19

Cancel()
{
    kill $pid0 $pid1
}
trap "Cancel" SIGINT SIGTERM

#Usage
if [[ $# -ne 2 ]] && [[ $# -ne 3 ]]; then
    echo "uc_check count [com1] [com2]"
    echo "if only one COM port was given, do loopback."
    echo "e.g.: ./uc_check.sh 10 ttyXR0 ttyXR1"
    exit 1
fi

count=$1
COM1=$2
if [[ $# -eq 3 ]]; then
    COM2=$3
    loopback=0
else
    loopback=1
fi

if [ -f "./uc" ] ;then
    UC="./uc"
else
    UC="uc"
fi

LOG_PATH="/dev/shm"

result=0
#Serial port default setting
baudrate=115200
bits=8
parity=0
stopbit=1

diff_result()
{
    [ $result -eq 1 ] && return

    local loopback=$1

    if [ $loopback -eq 1 ]; then
        diff $LOG_PATH/send_${COM1}.log $LOG_PATH/receive_${COM1}.log > /dev/null
        if [[ $? -ne 0 ]]; then
            result=1
        fi
    else
        diff $LOG_PATH/send_${COM1}.log $LOG_PATH/receive_${COM2}.log > /dev/null
        if [[ $? -ne 0 ]]; then
            result=1
        fi

        diff $LOG_PATH/send_${COM2}.log $LOG_PATH/receive_${COM1}.log > /dev/null
        if [[ $? -ne 0 ]]; then
            result=1
        fi
    fi
}

run_and_wait()
{
    [ $result -eq 1 ] && return

    local loopback=$1
    local count=$2

    if [ $loopback -eq 1 ]; then
        $UC both /dev/$COM1 $baudrate $bits $parity $stopbit $count > /dev/null &
        pid0=$!
        wait $pid0

    else
        $UC both /dev/$COM1 $baudrate $bits $parity $stopbit $count > /dev/null &
        pid0=$!

        $UC both /dev/$COM2 $baudrate $bits $parity $stopbit $count > /dev/null &
        pid1=$!

        wait $pid0
        wait $pid1
    fi
}

step=1000
loop=`expr $count / $step`
rest_count=`expr $count - $loop \* $step`

echo "Start serial port testing"
echo "Test start at $(date '+%Y/%m/%d %H:%M:%S')"
if [[ $loopback -eq 1 ]]; then
    echo "$COM1 loopback"
else
    echo "$COM1 <-> $COM2"
fi

echo "Wait for testing complete."
for ((i=0; i<$loop; i++)); do
    run_and_wait $loopback $step
    diff_result $loopback
done

if [ $rest_count -ne 0 ]; then
    run_and_wait $loopback $rest_count
    diff_result $loopback
fi

echo ""
echo "Test end at $(date '+%Y/%m/%d %H:%M:%S')"

if [[ $result -eq 0 ]]; then
    echo "PASS"
else
    echo "FAIL"
fi
