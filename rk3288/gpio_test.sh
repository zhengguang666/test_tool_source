#!/system/bin/sh
#Program

if [[ $1 -le 1 ]];then
echo "Usage: ./gpio_test.sh (GPIO-NUM) (LoopTimes)"
echo "For example: ./gpio_test.sh 219 100"
echo "             Test gpio219 high to low 100 times"
exit 1
fi

num=$1
Times=$2
i=1
echo $num > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio$num/direction
while ((i<$Times))
do
echo "Loop's "$i
echo 1 > /sys/class/gpio/gpio$num/value
echo "GPIO$num output 1"
usleep 10000
echo 0 > /sys/class/gpio/gpio$num/value
echo "GPIO$num output 0"
usleep 10000
i=$(($i+1))
done

