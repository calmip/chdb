#! /bin/bash

#
# USE: ext_cmd.sh input_file output_file [status]
#      

INPUT=$1
OUTPUT=$2
STS=$3

cat $INPUT |while read sts text
do
	echo -e "STS\t$sts"   > $OUTPUT || exit 10
	echo -e "TXT\t$text"  >>$OUTPUT || exit 10
	break
done

exit ${STS:-$(grep STS $OUTPUT |cut -f2)}


