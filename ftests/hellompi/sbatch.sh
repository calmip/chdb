#!/bin/bash
#SBATCH -N 1
#SBATCH -n 20

module purge 
module load intel/14.0.2.144 intelmpi/4.1.3.049 chdb/intelmpi/0.75

# 1/ Compiling program hello.cpp
mpicxx -o hello hello.cpp

# 2/ Exporting something, it is used by hello.cpp
export MA_VARIABLE_DENVIRONNEMENT=$(date)

# 3/ launching chdb on 6 cpus (1 master + 5 slaves, each slave is an mpi code, 2 processes/slave)
mpirun -n 6 chdb --mpi-slaves 2 --out-files toto --in-type txt --in-dir input --command  "./hello %in-dir%/%path% >%out-dir%/%path%" --report input.out/report.txt --on-error errors.txt --verbose

