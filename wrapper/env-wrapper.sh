#!/bin/bash

# Usage: 
# From chdb with the switch --mpi-slaves
#
#    env-wrapper.sh
#
#    Take the list of variables cited in CHDB_ENVIRONMENT, they SHOULD BE SET
#    Echo those variables, base64 encoded
#    Add the content of $CHDB_EXPORT_LIST
#
# EC, Calmip, 2017
#

#set -v

echo $(for p in $CHDB_ENVIRONMENT; do eval echo $p=\${$p}; done) "$CHDB_EXPORT_LIST" | base64 -w 0

