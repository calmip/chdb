# chdb

Thanks to chdb, it is easy to run an embarassingly parallel task on a gnu/linux cluster.

## How does this work ?
chdb works in client-server mode, using the mpi library to launch elementary tasks on several cores, or on 
several nodes of a cluster.

The server dispatches the work among the slaves and collects some information: exit status, time spent to execute, etc.

The tasks should be written as a single command line. An input file - or an input directory, is affected to each task. The output files may be kept in
an output directory. If the input files are in a hierarchy (some directories and subdirectories), the same hierarchy (ie the same directories and subdirectory names) 
is created back in the output. So it is easy to find the output corresponding to a certain input file.

The tasks launched by chdb may be:
- A Sequential program
- A multithreaded program using several cores of the node
- An mpi program. The only restriction is that each elementary tasks cannot use more than one cluster node.

You can have a look to our tutorial on the Calmip website: https://www.calmip.univ-toulouse.fr/espace-utilisateurs/doc-technique-olympe/lancer-un-calcul-sur-olympe/chdb-tutorial

