#!/bin/bash
dir=$PWD
cp main.c ../../Source/Src/User/main.c
cd ../../Source
make && make run
cd $dir
echo "/* Used as example, it will be changed in the future. */" > ../../Source/Src/User/main.c
echo "int main(void)" >> ../../Source/Src/User/main.c
echo "{"  >> ../../Source/Src/User/main.c
echo "    return 0;"  >> ../../Source/Src/User/main.c
echo "}" >> ../../Source/Src/User/main.c