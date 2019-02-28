#!/bin/sh
for i in 1 2 3 4 5 6 7 8 9 10
do
  sh "./waf --run \"cda 
    --capacity=$i
    --compressionEnabled=0\" >> logs.txt"
  sh "./waf --run \"cda 
    --capacity=$i
    --compressionEnabled=1\" >> logs.txt"
done