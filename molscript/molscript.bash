#!/bin/bash
#SBATCH -J molscript
#SBATCH -N 1
#SBATCH -n 20
#SBATCH --ntasks-per-node=20
#SBATCH --ntasks-per-core=1
### #SBATCH --mail-user=toto@mail.com
### #SBATCH --mail-type=ALL

#
# utilisation de chdb avec molauto/molscript pour generer un fichier postscript 
# pour chaque entree de la pdb
#
# Exemple de script utilisable avec slurm
#
# La commande suivante permet d'afficher le resultat:
# ghostscript $(find ps -type f)
#
# cf. http://www.avatar.se/molscript/
#

module purge
module load bullxmpi

export OMPI_MCA_ess=^pmi
export OMPI_MCA_pubsub=^pmi
export OMPI_MCA_mpi_leave_pinned=1

srun --resv-ports chdb --in-dir pdb --in-type ent --out-dir ps --out-files %out-dir%/%path% --sort-by-size --command "./molauto %in-dir%/%path% |./molscript -ps >%out-dir%/%dirname%/%basename%.ps"

