cmd_/home/seger/driverontwikkeling/WK1/3.8/Module.symvers := sed 's/\.ko$$/\.o/' /home/seger/driverontwikkeling/WK1/3.8/modules.order | scripts/mod/modpost -m -a  -o /home/seger/driverontwikkeling/WK1/3.8/Module.symvers -e -i Module.symvers   -T -
