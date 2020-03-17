#!/bin/bash

./build_i386.sh $1

if [ $? -ne 0 ]
then
    exit $?
fi 

qemu-system-i386 -cpu Nehalem -d guest_errors -rtc base=localtime -m 8G \
           -gdb tcp::1234 -smp 4 -serial stdio \
		   -drive format=raw,file=QEMU_Image.img 