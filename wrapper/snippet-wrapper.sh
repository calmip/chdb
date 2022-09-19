#!/bin/bash

# Usage: 
# From chdb with the switch --mpi-slaves
#
#    snippet-wrapper.sh
#
#    Base64 encode the environment variable $CHDB_SNIPPET
#
# EC, Calmip, 2017 - 2022
#

#set -v

echo $CHDB_SNIPPET | base64 -w 0
