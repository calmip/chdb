#!/bin/bash

#
# Usage (from chdb, using --mpi-slaves)
#
#    slave-wrapper.sh WORKDIR M64 MPI_SLAVES HOSTNAME COMMANDE PARAMETRES
#
#    - WORKDIR             --> We must change directory to $WORKDIR 
#    - M64                 --> the modules to load, base 64 coded (easier to manage strange characters) 
#	 - E64                 --> the environment variables to retrieve
#    - MPI_SLAVES          --> Number of mpi processes (s), may be also nb of processes/node and nb of threads/process (S:s:c)
#    - HOSTNAME            --> The hostname (right now only localhost suppported)
#    - COMMANDE PARAMETRES --> The command and its parameters
#
# EC, Calmip, 2015
#

#set -v

# Parameters
WORKDIR=$1; shift
M64=$1;shift
E64=$1;shift
MPI_SLAVES=$1; shift
HOSTNAME=$1; shift

# Load the modules: their names is base64 coded and stored in $M64.
module purge 2>/dev/null
for m in $(echo $M64|base64 -d); do module load $m 2> /dev/null ;done

# Print the modules only in verbose mode
[ -z "$CHDB_VERBOSE" ] || module li

# Set the environment variables
for v in $(echo $E64|base64 -d); do eval "export $v"; done

# Print the env only in verbose mode
[ -z "$CHDB_VERBOSE" ] || for v in CHDB_RANK CHDB_COMM_SIZE; do echo "$v=${!v}"; done

# Parse $MPI_SLAVES
#       If "4"    ==> numa="NA" (Non applicable) and $s=4
#       If "5:2:2 ==> numa=--physcpubind=0-3 for slave 1, 4-7 for slave 2, ..., 16-19 for slave 5, again 0-3 for slave 6 etc.
read numa s <<< $(echo $MPI_SLAVES | awk -F':' -v r=$CHDB_RANK 'NF==3{ S=$1;s=$2;c=$3;size=s*c;offset=((r-1)%S)*size;printf "%i-%i %i\n",offset,offset+size-1,s}NF==1 {print "NA",$1}')

# Comment out for debugging
#echo "WORKDIR=$WORKDIR"
#echo "MPI_SLAVES=$MPI_SLAVES ==> numa=$numa, s=$s"
#echo "HOSTNAME=$HOSTNAME"
#echo "COMMAND=""$@"

# go to workdir
cd $WORKDIR

# Calling mpirun, may be through numactl
if [[ $numa == "NA" ]]
then
#	echo mpirun -np $s -host $HOSTNAME "$@"
	mpirun -np $s -host $HOSTNAME "$@"
	exit $?
else
#	echo numactl --physcpubind $numa -- mpirun -np $s -host $HOSTNAME "$@"
	numactl --physcpubind $numa -- mpirun -np $s -host $HOSTNAME "$@"
	exit $?
fi
