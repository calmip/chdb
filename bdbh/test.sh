#! /bin/bash

# script de test de bdbh: 
# Usage: ./test.sh [REPERTOIRE] [DIRECTORIESPERLEVEL] [FILESPERDIR] [DEPTH]
#
# cree un repertoire avec des noms de fichiers et de repertoire aleatoires
# met cette hierarchie dans une BD
# extrait la base de donnees dans un nouveau repertoire
# compare les deux
#

ROOTDIR=${1-TESTDATA}
DIRECTORIESPERLEVEL=${2-10}
FILESPERDIR=${3-10}
DEPTH=${4-3}

BDBH_EXE="./bdbh"

RANDOMNAME=$(pwd)/random_name;

# creation d'une arborescence avec une profondeur $DEPTH
function files_and_dir()
{
    local l=${1-0}
    if (( l < $DEPTH ))
    then
	for d in $($RANDOMNAME 15 $DIRECTORIESPERLEVEL)
	do
	    mkdir $d >/dev/null 2>&1
	    ( cd $d; files_and_dir $(( $l+1 )) )
	done
	
	for f in $($RANDOMNAME 10 $FILESPERDIR)
	do
	    if [[ ! -d $f ]]
	    then
		echo $($RANDOMNAME 2000 3) > $f.txt
	    fi
	done
    fi
}



mkdir $ROOTDIR 
(
    cd $ROOTDIR
    files_and_dir
)

# creation de la base de données
time $BDBH_EXE -d ${ROOTDIR}.bd create compress
time $BDBH_EXE -d ${ROOTDIR}.bd add -r $ROOTDIR
###$BDBH_EXE -d ${ROOTDIR}.bd --root $(pwd) add -r $ROOTDIR

mkdir ${ROOTDIR}.restit
time $BDBH_EXE -d ${ROOTDIR}.bd --directory ${ROOTDIR}.restit --root $ROOTDIR extract

diff -rq ${ROOTDIR} ${ROOTDIR}.restit
