#!/bin/sh

set -x
# set -e

sudo rmmod -f mydev
sudo insmod mydev.ko

./writer Angus &
./reader 192.168.111.129 8888 /dev/my_dev
