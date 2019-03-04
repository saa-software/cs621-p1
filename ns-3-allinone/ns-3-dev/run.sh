#!/bin/bash
DIR=$(pwd)
for i in 1 2 3 4 5 6 7 8 9 10
do
  ./waf --run "cda --capacity=$i --compressionEnabled=0"
  ./waf --run "cda --capacity=$i --compressionEnabled=1"
done