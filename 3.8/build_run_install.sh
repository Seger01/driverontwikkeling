#!/bin/bash

sudo rm -f /dev/mydriver

sudo mknod /dev/mydriver c 500 0 -m 0666

make clean

make opgave_3_8.ko -j 16

sudo rmmod opgave_3_8

sudo insmod opgave_3_8.ko
