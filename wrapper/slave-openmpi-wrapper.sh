#!/bin/bash

#
# Usage (from chdb, using --mpi-slaves)
#
#    slave-wrapper.sh WORKDIR M64 MPI_SLAVES HOSTNAME COMMANDE PARAMETRES
#
#    - WORKDIR             --> We must change directory to $WORKDIR 
#    - M64                 --> the modules to load, base 64 coded (easier to manage strange characters) 
#    - E64                 --> the environment variables to retrieve
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

# Set the environment variables (including CHDB_VERBOSE if --verbose is specified)
#debug
echo $E64|base64 -d
for v in $(echo $E64|base64 -d)
do
   [ -z "$CHDB_VERBOSE" ] || echo "export $v" 
    eval "export $v"
done

# Print the env only in verbose mode
[ -z "$CHDB_VERBOSE" ] || for v in CHDB_RANK CHDB_COMM_SIZE; do echo "$v=${!v}"; done

[ -z "$CHDB_VERBOSE" ] || echo "====================================="

# Print the name of wrapper - only in verbose mode
[ -z "$CHDB_VERBOSE" ] || echo "slave wrapper = $0"

# Load the modules: their names is base64 coded and stored in $M64.
# If starts with '/' we suppose that it is a path to file to be sourced
# If does not start with '/' we suppose it is a module name

module purge 2>/dev/null
for m in $(echo $M64|base64 -d)
do
    if [[ "$m" =~ ^/ ]]
    then
        [ -z "$CHDB_VERBOSE" ] || echo "source $m"
        source $m
    else
        module load $m 2>/dev/null
    fi
done

# Print the modules only in verbose mode
[ -z "$CHDB_VERBOSE" ] || module li -t
[ -z "$CHDB_VERBOSE" ] || echo "====================================="

# Parse $MPI_SLAVES
#       If "4"    ==> numa="NA" (Non applicable) and $s=4
#       If "5:2:2 ==> numa=--physcpubind=0-3 for slave 1, 4-7 for slave 2, ..., 16-19 for slave 5, again 0-3 for slave 6 etc.
read numa s t <<< $(echo $MPI_SLAVES | awk -F':' -v r=$CHDB_RANK 'NF==3{ S=$1;s=$2;c=$3;size=s*c;offset=((r-1)%S)*size;for (k=0;k<size-1;k++) { printf "%i,",offset+k ; }; printf "%i",offset+k; printf " %i %i\n",s,c}NF==1 {print "NA",$1,$3}')                  

# Comment out for debugging
#echo "WORKDIR=$WORKDIR"
#echo "CHDB_RANK=$CHDB_RANK"
#echo "MPI_SLAVES=$MPI_SLAVES ==> numa=$numa, s=$s"
#echo "HOSTNAME=$HOSTNAME"
#echo "COMMAND=""$@"

# go to workdir
cd $WORKDIR

# Calling mpirun, may be through numactl
if [[ $numa == "NA" ]]
then
    [ -z "$CHDB_VERBOSE" ] || echo mpirun -np $s "$@"
    mpirun -np $s "$@"
else
    [ -z "$CHDB_VERBOSE" ] || echo "Which mpirun --> " $(which mpirun)
    [ -z "$CHDB_VERBOSE" ] || echo mpirun -np $s --cpu-set $numa "$@"
    mpirun -np $s --cpu-set $numa "$@"
fi
