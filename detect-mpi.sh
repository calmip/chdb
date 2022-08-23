#! /bin/bash
# 
# This script tries to detect the MPI flavour loaded on this machine
#
# We run mpiexec --version and analyze the output
# Only openmpi and intelmpi products are currentliy detected
#
# Usage: detect-mpi.sh
#
#        The result is sent to stdout:
#            OPENMPI     openmpi detected
#            INTELMPI    intelmpi detected
#            OTHER       mpi detected, but don't know the version
#	     NONE        no mpi detected (ie mpiexec not in the path)
#

MPIEXEC_OUTPUT=$(mpiexec --version 2>/dev/null)
MPIEXEC_STATUS=$?

# mpiexec not found, no mpi installed or loaded
if [ "$MPIEXEC_STATUS" = "127" ]
then
   /bin/echo -n NONE
   exit 0
fi

if grep -qiF "Intel" <<< $MPIEXEC_OUTPUT
then
   /bin/echo -n INTELMPI
   exit 0
fi

if grep -qiF "open-mpi" <<< $MPIEXEC_OUTPUT
then
   /bin/echo -n OPENMPI
   exit 0
fi

# mpiexec found, but we did not undestrand the version
/bin/echo -n OTHER
exit 0

