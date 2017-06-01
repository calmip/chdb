#!/bin/bash

# Utilisation (a partir de chdb avec le switch --mpi-slaves)
#
#    slave-wrapper.sh WORKDIR M64 MPI_SLAVES HOSTNAME COMMANDE PARAMETRES
#
#    - WORKDIR --> faire un chdir $WORKDIR 
#    - M64     --> Les modules a charger, codes en base64 (pour eviter les histoires de guillemets)
#    - MPI_SLAVES          --> Combien de processes mpi voulons-nous lancer
#    - HOSTNAME            --> nom du host (pour l'instant un seul, localhost est le meilleur choix)
#    - COMMANDE PARAMETRES --> La commande mpi a lancer
#
# EC, Calmip, 2015
#

#set -v

# Passage de paramètres
WORKDIR=$1; shift
M64=$1;shift
MPI_SLAVES=$1; shift
HOSTNAME=$1; shift

# Charger les modules: le nom des modules est code en base64 dans la variable $M64
module purge
for m in $(echo $M64|base64 -d); do module load $m; done

# Imprimer les modules pour verification
module li

#echo "WORKDIR=$WORKDIR"
#echo "MPI_SLAVES=$MPI_SLAVES"
#echo "HOSTNAME=$HOSTNAME"
#echo "$@"

# cd 
cd $WORKDIR

# Appel de mpirun
mpirun -np $MPI_SLAVES -host $HOSTNAME "$@"

