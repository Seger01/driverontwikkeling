#/bin/bash

sudo rm -f /dev/mydriver

sudo mknod /dev/mydriver c 500 0 -m 0666

make clean

make mymodule.ko -j 16

sudo rmmod mymodule

sudo insmod mymodule.ko

make clean
