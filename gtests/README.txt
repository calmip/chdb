Running the tests for chdb:
===========================

1/ The following binaries should be available (may be a symlink)

./chdb.exe
./bdbh
   
2/ Building the tests:
make
   
3/ Running the tests:
./parameters_unittest 
./buffer_unittest 
rm -rf input*; ./directories_unittest 
mpirun -n 2 ./scheduler_unittest
./system_unittest 
rm -rf input*; ./chdb_unittest 

4/ Please note the use of mpirun -v 2 for running scheduler_unittest
5/ ./directories_unittest MAY fail (1 test) on nfs-based filesystems, it should work on Lustre or gpfs filesystems.



   
   
