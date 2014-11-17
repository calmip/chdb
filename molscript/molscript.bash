#!/bin/bash
#SBATCH -J molscript
#SBATCH -N 1
#SBATCH -n 20
#SBATCH --ntasks-per-node=20
#SBATCH --ntasks-per-core=1
### #SBATCH --mail-user=toto@mail.com
### #SBATCH --mail-type=ALL

module purge
module load intel intelmpi

#
# utilisation de chdb avec molauto/molscript pour generer un fichier postscript 
# pour chaque entree de la pdb
#
# Exemple de script utilisable avec slurm sur eos
#
# La commande suivante permet d'afficher le resultat:
# ghostscript $(find ps -type f)
#
# cf. http://www.avatar.se/molscript/
#

mpirun chdb --in-dir pdb --in-type ent --out-dir ps --out-files %out-dir%/%path% --sort-by-size --command "./molauto %in-dir%/%path% |./molscript -ps >%out-dir%/%dirname%/%basename%.ps"

