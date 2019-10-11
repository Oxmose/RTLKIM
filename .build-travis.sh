#!/bin/bash

cd Tests
chmod +x ./unit_tests_i386.sh
./unit_tests_i386.sh
val=$?
cd ..
exit $val