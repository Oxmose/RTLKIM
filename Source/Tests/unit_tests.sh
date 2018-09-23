#!/bin/bash

error=0
success=0

for entry in "./Tests"/*.c
do
    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}

    echo -e "\e[94m################### $filename_up test\e[39m"
    # Select the test
    sed -i "s/$filename_up 0/$filename_up 1/g" Tests/test_bank.h
    sed -i 's/TEST_MODE_ENABLED 0/TEST_MODE_ENABLED 1/g' ../Config/config.h
    {
        # Execute the test
        rm -f *.out
        cd ../
        make && (make qemu-test-mode > test.out &)
        sleep 2
        killall qemu-system-i386
        mv test.out Tests/test.out
        cd Tests
    } > /dev/null
    # Filter output
    grep '\[TESTMODE\]' test.out > filtered.out
    #Compare output
    diff filtered.out Refs/$filename.valid >> /dev/null
    if (( $? != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error=$((error + 1))
    else
        echo -e "\e[92mPASSED\e[39m"
        success=$((success + 1))
    fi
    #Clean data
    rm *.out
    rm Tests/*.o
    #Restore non testmode
    sed -i "s/$filename_up 1/$filename_up 0/g" Tests/test_bank.h
    sed -i 's/TEST_MODE_ENABLED 1/TEST_MODE_ENABLED 0/g' ../Config/config.h
done

echo ""
echo -e "\e[94m################################### RESULTS ###################################\e[39m"
echo ""
if (( error != 0 ))
then
    echo -e "\e[31m $error ERRORS \e[39m"
    echo -e "\e[92m $success SUCCESS \e[39m"
    exit -1
else
    echo -e "\e[92m 0 ERROR \e[39m"
    echo -e "\e[92m $success SUCCESS \e[39m"
fi