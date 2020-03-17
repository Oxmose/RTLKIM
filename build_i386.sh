#!/bin/bash

echo -e "\e[1m\e[34m============ Building UTK I386 ============\e[22m\e[39m"
echo

if [ "$#" -ne 1 ]; then
    echo -e "\e[1m\e[31mUsage: $0 [Kernel name]\e[22m\e[39m"
    exit -1
fi

rm -rf build
mkdir -p build 

#####################################
# Building Kernel
#####################################
cd Source
#make clean
make arch=i386 

# Copy kernel 
objcopy -O binary Bin/kernel.bin ../build/kernel.bin

#####################################
# Generate bootloader configuration
#####################################
echo -e "\e[1m\e[34m============ Generating Bootloader Configuration ============\e[22m\e[39m"
cd ../Bootloader/i386 

k_size=$(wc -c < ../../build/kernel.bin)
k_size_sect=$(( $k_size / 512  * 2))
k_left=$(( $k_size % 512 ))
if [ $k_left -ne 0 ]
then 
    k_size_sect=$(( $k_size_sect + 1 ))
fi
echo "Kernel size: $k_size ($k_size_sect)"
echo "[bits 32]"                              > src/configuration.s 
echo "dd $k_size_sect       ; Kernel size in sectors" >> src/configuration.s 
echo "dd 0x00100000 ; Kernel start address"   >> src/configuration.s 
echo "dd 0x00100000 ; Kernel entry point"     >> src/configuration.s 
echo "dd $(( ${#1} + 1 )) ; Kernel name size"       >> src/configuration.s 
echo "db \"$1\", 0     ; Kernel name"            >> src/configuration.s 
echo "times 510-(\$-\$\$) db 0xFF"               >> src/configuration.s 
echo "dw 0xE621"                              >> src/configuration.s 

#####################################
# Compile bootloader
#####################################
echo -e "\e[1m\e[34m============ Compiling Bootloader ============\e[22m\e[39m"
make

echo -e "\e[1m\e[34m============ Merging Images ============\e[22m\e[39m"
cp bin/bootloader.bin ../../build/bootloader.bin 
cd ../../build 
cat bootloader.bin kernel.bin > os_image.bin
cp os_image.bin ../Os_Image.bin

echo -e "\e[1m\e[34m============ Creating QEMU disk ============\e[22m\e[39m"
qemu-img create -f raw os_image.img 64M
dd status=noxfer conv=notrunc if=os_image.bin of=os_image.img
cp os_image.img ../QEMU_Image.img

cd ..