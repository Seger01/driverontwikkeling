#/bin/bash

rm bbb.dts.pre
# preprocessing
cpp -nostdinc -I ./include -undef -x assembler-with-cpp ./src/arm/am335x-boneblack-uboot-univ.dts bbb.dts.pre

cp bbb.dts.pre input.dts

