module load chdb/intelmpi/0.75

export I_MPI_PMI_LIBRARY=/usr/lib64/libpmi.so


# 1/ Compiling program hello.cpp
c++ -o hello hello.cpp

# 2/ Exporting something, it is used by hello.cpp
export MA_VARIABLE_DENVIRONNEMENT=$(date)

# 3/ Creating a bdbh database from the input directory
rm -r input.db
bdbh create --database input.db
bdbh --database input.db add -r input/*

# 3/ launching chdb on 3 cpus (1 master + 2 slaves)
#mpirun -n 6 chdb --out-files toto --in-type txt --in-dir input.db --command  "./hello %in-dir%/%path% >%out-dir%/%path%" --report report.txt --verbose
srun -n 6 chdb --out-files %out-dir%/%path% --in-type txt --in-dir input.db --command  "./hello %in-dir%/%path% >%out-dir%/%path%" --report report.txt --verbose

