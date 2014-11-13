#
# utilisation de chdb avec molauto/molscript pour generer un fichier postscript 
# pour chaque entree de la pdb
#
# Exemple de script utilisable avec mpirun
#
# La commande suivante permet d'afficher le resultat:
# ghostscript $(find ps -type f)
#
# cf. http://www.avatar.se/molscript/
#

module purge
module load bullxmpi

mpirun -n 4 chdb --in-dir pdb --in-type ent --out-dir ps --out-files %out-dir%/%dirname%/%basename%.ps --command "./molauto %in-dir%/%path% |./molscript -ps >%out-dir%/%dirname%/%basename%.ps"

