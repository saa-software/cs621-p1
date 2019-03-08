#!/bin/bash
DIR=$(pwd)
for i in 1 2 3 4 5 6 7 8 9 10
do
  ./waf --run "scratch/cda --capacity=$i --compressionEnabled=0"
  ./waf --run "scratch/cda --capacity=$i --compressionEnabled=1"
done