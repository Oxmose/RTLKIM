dir=$PWD
cp main.c ../../Src/User/main.c
cd ../../
make && make run
cd $dir
echo "/* Used as example, it will be changed in the future. */" > ../../Src/User/main.c
echo "int main(void)" >> ../../Src/User/main.c
echo "{"  >> ../../Src/User/main.c
echo "    return 0;"  >> ../../Src/User/main.c
echo "}" >> ../../Src/User/main.c