#/bin/bash

cp input.dts ./src/arm/

dtc -I dts -O dtb -o output.dtb ./src/arm/input.dts 
