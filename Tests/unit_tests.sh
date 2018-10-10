#!/bin/bash

error=0
success=0
total=0
i=1
for entry in "./Tests"/*.c
do
    total=$((total + 1))
done
for entry in "./Tests"/*.c
do
    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}

    echo -e "\e[94m################### Test $i/$total : $filename_up\e[39m"
    # Select the test
    sed -i "s/$filename_up 0/$filename_up 1/g" Tests/test_bank.h
    sed -i 's/TEST_MODE_ENABLED 0/TEST_MODE_ENABLED 1/g' ../Source/Config/config.h
    {
        # Execute the test
        rm -f *.out
        cd ../Source
        make TESTS=TRUE && (make qemu-test-mode > test.out &)
        sleep 4
        killall qemu-system-x86_64
        mv test.out ../Tests/test.out
        cd ../Tests
    } > /dev/null
    # Filter output
    grep "\[TESTMODE\]\|ERROR" test.out > filtered.out
    #Compare output
    diff filtered.out Refs/$filename.valid >> /dev/null
    if (( $? != 0 ))
    then
        echo -e "\e[31mERROR \e[39m"
        error=$((error + 1))
        mv filtered.out $filename.error
    else
        echo -e "\e[92mPASSED\e[39m"
        success=$((success + 1))
    fi
    #Clean data
    rm *.out
    rm Tests/*.o
    #Restore non testmode
    sed -i "s/$filename_up 1/$filename_up 0/g" Tests/test_bank.h
    sed -i 's/TEST_MODE_ENABLED 1/TEST_MODE_ENABLED 0/g' ../Source/Config/config.h

    i=$((i + 1))
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