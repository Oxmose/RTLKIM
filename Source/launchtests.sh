#!/bin/bash
cd Tests
./unit_tests.sh
val=$?
cd ..
exit $val