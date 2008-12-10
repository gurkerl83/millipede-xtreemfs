#!/bin/bash
. tests/utilities.inc.sh

echo "running iozone in auto and diagnostic mode..."

for volume in $VOLUMES
do
	echo "iozone on volume $volume..."
	dir=`pwd`
	cd $volume
	/home/bjko/test/iozone3_283/src/current/iozone -a -+d
	if [ $? -ne 0 ]
	then
		cd $dir
		echo "ERROR: IOZone failed on $volume!"
		print_summary_message 1 "IOZone" $volume
		exit 1
	fi
	print_summary_message 0 "IOZone" $volume
	cd $dir
done