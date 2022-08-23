#!/bin/bash

# Usage: 
# From chdb with the switch --mpi-slaves
#
#    mod-wrapper.sh
#
#    Echo the output of module li, base64 encoded
#
# EC, Calmip, 2017
#
	
#set -v

module li -t 2>&1|tail -n +2|base64 -w 0
	
