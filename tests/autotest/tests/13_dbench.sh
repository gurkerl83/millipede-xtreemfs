#!/bin/bash
. tests/utilities.inc.sh

echo "running dbench with 25 clients on direct_io volumes ..."

for volume in $VOLUMES
do
	echo "dbench on volume $volume..."
	dbench-3.04/dbench -c dbench-3.04/client.txt -D $volume 5

	if [ $? -ne 0 ]
	then
		echo "ERROR: dbench failed on $volume!"
		print_summary_message 1 "DBench" $volume
		exit 1
	fi
	print_summary_message 1 "DBench" $volume
done
exit 0