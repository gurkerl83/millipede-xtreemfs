#!/bin/bash
. tests/utilities.inc.sh

echo "erichs data integrity test..."

for volume in $VOLUMES
do
	echo "writing 20x1MB files with marked data in $volume, checking data "
	$XTREEMFS/client/tests/suite/marked_block.pl --start=1 --nfiles=20 --size=1 \
	--group=10 --base=$volume/integritytest
	if [ $? -ne 0 ]; then
		echo "test failed!"
		print_summary_message 1 'data_integrity' $volume
		exit 1;
	fi
	print_summary_message 0 'data_integrity' $volume
done