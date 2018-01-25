source /opt/intel/composer_xe_2013_sp1.2.144/bin/compilervars.sh intel64

icc -openmp -mmic -O3 -fp-model fast=2  main.c qdbmp.c


