#
# EC, Calmip, 2015
#

#set -v
# export CHDB_VERBOSE=1

# Parameters
WORKDIR=$1; shift
M64=$1;shift
E64=$1;shift
MPI_SLAVES=$1; shift
HOSTNAME=$1; shift

# Load the modules: their names is base64 coded and stored in $M64.
 module purge  >/dev/null 2>&1
  for m in $(echo $M64|base64 -d); do module load $m; done  >/dev/null 2>&1

# For openmpi !
ulimit -s 10240

# Print the modules only in verbose mode
[ -z "$CHDB_VERBOSE" ] || module li

# Set the environment variables
for v in $(echo $E64|base64 -d); do eval "export $v"; done

# Print the env only in verbose mode
[ -z "$CHDB_VERBOSE" ] || for v in CHDB_RANK CHDB_COMM_SIZE; do echo "$v=${!v}"; done

# Parse $MPI_SLAVES
#       If "4"    ==> numa="NA" (Non applicable) and $s=4
#       If "5:2:2 ==> numa=--physcpubind=0-3 for slave 1, 4-7 for slave 2, ..., 16-19 for slave 5, again 0-3 for slave 6 etc.
read numa s t <<< $(echo $MPI_SLAVES | awk -F':' -v r=$CHDB_RANK 'NF==3{ S=$1;s=$2;c=$3;size=s*c;offset=((r-1)%S)*size;for (k=0;k<size-1;k++) { printf "%i,",offset+k ; }; printf "%i",offset+k; printf " %i %i\n",s,c}NF==1 {print "NA",$1,$3}')                  

# Comment out for debugging
#echo "WORKDIR=$WORKDIR"
#echo "MPI_SLAVES=$MPI_SLAVES ==> numa=$numa, s=$s, t=$t"
#echo "HOSTNAME=$HOSTNAME"
#echo "COMMAND=""$@"

# go to workdir
cd $WORKDIR

# Calling mpirun, may be through numactl
if [[ $numa == "NA" ]]
then
	#echo mpirun -np $s "$@"
	mpirun -np $s  "$@"
else
        #echo mpirun -np $s --cpu-set $numa --map-by slot --cpus-per-proc $t --report-bindings "$@"
        #mpirun -np $s --cpu-set $numa --map-by slot --cpus-per-proc $t --report-bindings "$@"
        # Use --report-bindings to see the binding 
        # echo mpirun -np $s --cpu-set $numa "$@"
        mpirun -np $s --cpu-set $numa "$@"
fi
