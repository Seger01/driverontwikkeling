#/bin/bash

rm bbb.dts.pre
# preprocessing
cpp -nostdinc -I ./include -undef -x assembler-with-cpp ./src/arm/am335x-boneblack.dts bbb.dts.pre

cp bbb.dts.pre input.dts

cp input.dts ./src/arm/

dtc -I dts -O dtb -o output.dtb ./src/arm/input.dts 

