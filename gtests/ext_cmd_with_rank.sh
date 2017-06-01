#! /bin/bash

#
# USE: ext_cmd.sh input_file output_file [status]
#      

INPUT=$1
OUTPUT=$2
STS=$3

if [[ ! -f $INPUT ]]
then
	exit 20
fi
if [[ -d $OUTPUT ]]
then
	exit 30
fi


cat $INPUT |while read sts text
do
	echo -e "RANK\t$CHDB_RANK"       >$OUTPUT || exit 10
	echo -e "SIZE\t$CHDB_COMM_SIZE" >>$OUTPUT || exit 10
	echo -e "STS\t$sts"             >>$OUTPUT || exit 10
	echo -e "TXT\t$text"            >>$OUTPUT || exit 10
	break
done

exit ${STS:-$(grep STS $OUTPUT |cut -f2)}


