#! /bin/bash
#
# Installation script for chdb:
#    ./install.sh ~ (for debugging)
#    ./install.sh /usr/local (for the prod)

USAGE="Usage: ./install ~|/usr/local ~/privatemodules|/usr/local/modules/modulefiles intelmpi|bullxmpi"
[ "$1" = "" ] && echo $USAGE && exit 1
[ "$2" = "" ] && echo $USAGE && exit 1
[ "$3" = "" ] && echo $USAGE && exit 1

# *DST = The top directories for the installation
BINDST=$1
MODDST=$2
MPI=$3
VERSION=$(awk -F'"' '/CHDB_VERSION/ {print $2}' version.hpp )

# *SRC = The SOURCE directories
# *SRCF= Some SOURCE files
BINSRC="."
MODSRC="modulefiles/chdb/$MPI"
WRAPSRC="wrapper/$MPI"
MODSRCF="$MODSRC/$VERSION"

# *DIR = The DESTINATION directories
# *DIRF= some DESTINATION files
BINDIR="$BINDST/local/chdb/$MPI/$VERSION/bin"
MODDIR="$MODDST/chdb/$MPI"
MODDIRF="$MODDIR/$VERSION"

# Checking the directories and important files do exist
[ ! -f "$MODSRCF" ] && echo "ERROR - $MODSRCF DOES NOT EXIST" && exit 1
[ ! -d "$MODDIR"  ] && echo "ERROR - $MODDIR  DOES NOT EXIST" && exit 1
[ ! -d "$BINDIR"  ] && echo "ERROR - $BINDIR  DOES NOT EXIST" && exit 1

# Comment out if compiled with bdbh !!!!!
#BDBH=bdbh/bdbh

cp $BDBH "chdb.exe" "$WRAPSRC/chdb" "$WRAPSRC/slave-wrapper.sh" "$WRAPSRC/env-wrapper.sh" "$WRAPSRC/mod-wrapper.sh" "$BINDIR"
( cd $BINDIR; chmod 755 $BDBH chdb chdb.exe *-wrapper.sh )

sed -e "s!BINDIR!$BINDIR!g" <$MODSRCF >$MODDIRF
chmod 644 $MODDIRF

