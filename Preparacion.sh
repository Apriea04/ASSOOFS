#!/bin/bash

#Script para montar ASSOOFS

umount mnt/
rmmod assoofs
rm -r mnt

make clean

make
dd bs=4096 count=100 if=/dev/zero of=image
./mkassoofs image
insmod assoofs.ko
mkdir mnt
mount -o loop -t assoofs image mnt
cd mnt

echo Listo
