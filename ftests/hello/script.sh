
# 1/ Compiling program hello.cpp
c++ -o hello hello.cpp

# 2/ Exporting something, it is used by hello.cpp
export MA_VARIABLE_DENVIRONNEMENT=$(date)

# 3/ launching chdb on 3 cpus (1 master + 2 slaves)
mpirun -n 3 chdb --out-files toto --in-type txt --in-dir input --out-dir output.3 --command  "./hello %in-dir%/%path% >%out-dir%/%path%" --report output.3/report.txt

# 4/ launching chdb on 11 cpus (1 master + 10 slaves)
mpirun -n 10 chdb --out-files toto --in-type txt --in-dir input --out-dir output.10 --command  "./hello %in-dir%/%path% >%out-dir%/%path%" --report output.10/report.txt
