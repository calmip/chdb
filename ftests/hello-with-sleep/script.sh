#export I_MPI_PMI_LIBRARY=/usr/lib64/libpmi.so


# 1/ Compiling program hello.cpp
c++ -o hello hello.cpp

# 2/ Exporting something, it is used by hello.cpp
export MA_VARIABLE_DENVIRONNEMENT=$(date)

# 3/ launching chdb on 3 cpus (1 master + 2 slaves)
mpirun -n 6 chdb --out-files toto --in-type txt --in-dir input --command  "./hello %in-dir%/%path% >%out-dir%/%path%" --report input.out/report.txt --sleep 2
#srun -n 6 chdb --out-files toto --in-type txt --in-dir input --command  "./hello %in-dir%/%path% >%out-dir%/%path%" --report input.out/report.txt

