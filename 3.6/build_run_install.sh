#!/bin/bash

sudo rm -f /dev/mydriver

sudo mknod /dev/mydriver c 500 0 -m 0666

make clean

make opgave_3_6.ko -j 16

sudo rmmod opgave_3_6

sudo insmod opgave_3_6.ko
