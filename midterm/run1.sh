#!/bin/bash
for ((i = 0; i < 10; i++))
do
  ./client -s '/tmp/myfifo2' -o 'readfile' & 
done
wait