#!/bin/bash

cd Tests
chmod +x ./unit_tests.sh
./unit_tests.sh
val=$?
cd ..
exit $val