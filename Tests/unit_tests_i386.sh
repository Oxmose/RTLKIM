#!/bin/bash

function testcase() {
    entry=$1

    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}

    echo -e "\e[94m################### Test $i/$total : $filename_up ($2) \e[39m"
    # Select the test
    sed -i "s/$filename_up 0/$filename_up 1/g" Tests/test_bank.h
    sed -i 's/TEST_MODE_ENABLED 0/TEST_MODE_ENABLED 1/g' ../Source/Config/i386/config.h
    {
        # Execute the test
        rm -f *.out
        cd ../Source
        make arch=i386 TESTS=TRUE && (make arch=i386 qemu-test-mode > test.out &)
        sleep $2
        killall qemu-system-i386
        killall make
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
        cat filtered.out
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
    sed -i 's/TEST_MODE_ENABLED 1/TEST_MODE_ENABLED 0/g' ../Source/Config/i386/config.h
}

error=0
success=0
total=0
i=1

TIMES=(2 2 4 2 2 2 2 2 2 2 2\
       2 2 2 4 4 4 2 3 2 2 2\
       2 6 5 4 4 3 4 4 4 2 4)
for entry in "./Tests"/*.c
do
    total=$((total + 1))

    filename=$(basename -- "$entry")
    filename="${filename%.*}"
    filename_up=${filename^^}
    sed -i "s/$filename_up 1/$filename_up 0/g" Tests/test_bank.h
done

for entry in "./Tests"/*.c
do
    testcase $entry ${TIMES[ $(( i - 1 )) ]}

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
