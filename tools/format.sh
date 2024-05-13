#!/bin/sh

set -e

for FILE in $(find src -name *.cpp -o -name *.h)
do
    clang-format -i $FILE
done
