cmd_/home/seger/driverontwikkeling/3.5/Module.symvers := sed 's/\.ko$$/\.o/' /home/seger/driverontwikkeling/3.5/modules.order | scripts/mod/modpost -m -a  -o /home/seger/driverontwikkeling/3.5/Module.symvers -e -i Module.symvers   -T -
