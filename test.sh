#!/bin/bash

read inputData

# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
./build/demo/vadec_s ./res/GEN-Chn-0-STU/cfg $inputData ./output.txt
