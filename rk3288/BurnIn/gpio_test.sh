#!/system/bin/sh
#Program

echo 219 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio219/direction
usleep 1000
echo 220 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio220/direction
usleep 1000
echo 221 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio221/direction
usleep 1000
echo 237 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio237/direction
usleep 1000
echo 250 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio250/direction
usleep 1000
echo 251 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio251/direction
usleep 1000
echo 248 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio248/direction
usleep 1000
echo 249 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio249/direction

while true
do
    echo 1 > /sys/class/gpio/gpio219/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio219/value
    usleep 1000
    echo 1 > /sys/class/gpio/gpio220/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio220/value
    usleep 1000
    echo 1 > /sys/class/gpio/gpio221/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio221/value
    usleep 1000
    echo 1 > /sys/class/gpio/gpio237/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio237/value
    usleep 1000
    echo 1 > /sys/class/gpio/gpio250/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio250/value
    usleep 1000
    echo 1 > /sys/class/gpio/gpio251/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio251/value
    usleep 1000
    echo 1 > /sys/class/gpio/gpio248/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio248/value
    usleep 1000
    echo 1 > /sys/class/gpio/gpio249/value
    usleep 1000
    echo 0 > /sys/class/gpio/gpio249/value
    usleep 1000
done
