#!/bin/bash
#make clean
make -j4
#filename=$1
filename="mytest.seal"
echo "--------Test using" $filename "--------"
name=${filename//.seal}
./cgen $filename -o $name.s
./cgen_answer $filename -o "output.s"
gcc $name.s -no-pie -o $name
#make clean