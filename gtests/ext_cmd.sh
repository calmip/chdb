#! /bin/bash

#
# USE: ext_cmd.sh input_file output_file
#      

INPUT=$1
OUTPUT=$2

while read sts text
do
	echo "STS $sts"   > $OUTPUT || exit 10
	echo "TXT $text"  >>$OUTPUT || exit 10
	break
done <$INPUT

exit $sts
