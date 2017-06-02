#! /bin/bash
#
# Installation script for chdb:
#    ./install.sh ~ (for debugging)
#    ./install.sh /usr/local (for the prod)

[ "$1" = "" ] && echo "Usage: ./install directory default|intelmpi|bullxmpi" && exit 1
[ "$2" = "" ] && echo "Usage: ./install directory default|intelmpi|bullxmpi" && exit 1

DST=$1
MPI=$2
VERSION=$(awk '/CHDB_VERSION/ {print $3}' version.hpp )

EXE="chdb.exe"
CHDB=chdb
WRAPPER='slave-wrapper.sh'
WRAPDIR="slave-wrapper/$MPI/"

BINDIR="$DST/local/chdb/$MPI/$VERSION/bin"

# TODO - Arranger ca pour la prod !!!
MODULEDIR=~/privatemodules/chdb/$MPI
MODULEFILE="$MODULEDIR/$VERSION"
MODULESRCFILE="modulefiles/chdb/$MPI/VERSION"

cp "$EXE" "$WRAPDIR/$CHDB" "$WRAPDIR/$WRAPPER" "$BINDIR"
sed -e "s!BINDIR!$BINDIR!" -e 's/VERSION/$VERSION/' <$MODULESRCFILE >$MODULEFILE

( cd $BINDIR; chmod 755 $EXE $CHDB $WRAPPER )
chmod 644 $MODULEFILE


