#!/bin/bash

osd_config() {
	osdnum=$1
	OUTPUT="$TEST_DIR/config/osd$osdnum.config"
	tmp_date=`date`
	echo "#generated by XtreemFS auto tests ($tmp_date)" > $OUTPUT
	echo "uuid = test-env-OSD$osdnum" >> $OUTPUT
	echo "debug_level = $DEBUG" >> $OUTPUT
	let tmp=$OSD_START_PORT+$osdnum
	echo "listen.port = $tmp" >> $OUTPUT
	let tmp=30640+$osdnum
	echo "http_port = $tmp" >> $OUTPUT
	echo "dir_service.host = localhost" >> $OUTPUT
	echo "dir_service.port = $DIR_PORT" >> $OUTPUT
	echo "object_dir = $TEST_DIR/data/osd$osdnum/" >> $OUTPUT
	echo "local_clock_renewal = 50" >> $OUTPUT
	echo "remote_time_sync = 60000" >> $OUTPUT
	echo "report_free_space = true" >> $OUTPUT
        echo "checksums.enabled = false" >> $OUTPUT
        echo "capability_secret = testsecret" >> $OUTPUT
	if [ $SSL_ENABLED -eq 0 ]
	then
		echo "ssl.enabled = false" >> $OUTPUT
	else
		echo "ssl.enabled = true" >> $OUTPUT
		echo "ssl.service_creds = $CERT_DIR/OSD.p12" >> $OUTPUT
		echo "ssl.service_creds.pw = passphrase" >> $OUTPUT
		echo "ssl.service_creds.container = PKCS12" >> $OUTPUT
		echo "ssl.trusted_certs = $CERT_DIR/trusted.jks" >> $OUTPUT
		echo "ssl.trusted_certs.pw = passphrase" >> $OUTPUT
		echo "ssl.trusted_certs.container = JKS" >> $OUTPUT
	fi
}

mrc_config() {
	OUTPUT="$TEST_DIR/config/mrc.config"
	tmp_date=`date`
	echo "#generated by XtreemFS auto tests ($tmp_date)" > $OUTPUT
	echo "uuid = test-env-MRC" >> $OUTPUT
	echo "debug_level = $DEBUG" >> $OUTPUT
	echo "listen.port = $MRC_PORT" >> $OUTPUT
	echo "http_port = 30636" >> $OUTPUT
	echo "dir_service.port = $DIR_PORT" >> $OUTPUT
	echo "dir_service.host = localhost" >> $OUTPUT
	echo "database.log = $TEST_DIR/data/mrc/" >> $OUTPUT
	echo "database.dir = $TEST_DIR/data/mrc/" >> $OUTPUT
	echo "osd_check_interval = 300" >> $OUTPUT
	echo "no_atime = true" >> $OUTPUT
	echo "no_fsync = true" >> $OUTPUT
	echo "local_clock_renewal = 50" >> $OUTPUT
	echo "remote_time_sync = 60000" >> $OUTPUT
        echo "database.checkpoint.interval = 1800000" >> $OUTPUT
        echo "database.checkpoint.idle_interval = 1000" >> $OUTPUT
        echo "database.checkpoint.logfile_size = 16384" >> $OUTPUT
	echo "authentication_provider = org.xtreemfs.common.auth.NullAuthProvider" >> $OUTPUT
	echo "capability_secret = testsecret" >> $OUTPUT
	if [ $SSL_ENABLED -eq 0 ]
	then
		echo "ssl.enabled = false" >> $OUTPUT
	else
		echo "ssl.enabled = true" >> $OUTPUT
		echo "ssl.service_creds = $CERT_DIR/MRC.p12" >> $OUTPUT
		echo "ssl.service_creds.pw = passphrase" >> $OUTPUT
		echo "ssl.service_creds.container = PKCS12" >> $OUTPUT
		echo "ssl.trusted_certs = $CERT_DIR/trusted.jks" >> $OUTPUT
		echo "ssl.trusted_certs.pw = passphrase" >> $OUTPUT
		echo "ssl.trusted_certs.container = JKS" >> $OUTPUT
	fi
}


dir_config() {
	OUTPUT="$TEST_DIR/config/dir.config"
	tmp_date=`date`
	echo "#generated by XtreemFS auto tests ($tmp_date)" > $OUTPUT
	echo "uuid = test-env-DIR" > $OUTPUT
	echo "debug_level = $DEBUG" >> $OUTPUT
	echo "listen.port = $DIR_PORT" >> $OUTPUT
        echo "http_port = 30638" >> $OUTPUT
	echo "database.dir = $TEST_DIR/data/dir/" >> $OUTPUT
	echo "authentication_provider = org.xtreemfs.common.auth.NullAuthProvider" >> $OUTPUT
	if [ $SSL_ENABLED -eq 0 ]
	then
		echo "ssl.enabled = false" >> $OUTPUT
	else
		echo "ssl.enabled = true" >> $OUTPUT
		echo "ssl.service_creds = $CERT_DIR/DIR.p12" >> $OUTPUT
		echo "ssl.service_creds.pw = passphrase" >> $OUTPUT
		echo "ssl.service_creds.container = PKCS12" >> $OUTPUT
		echo "ssl.trusted_certs = $CERT_DIR/trusted.jks" >> $OUTPUT
		echo "ssl.trusted_certs.pw = passphrase" >> $OUTPUT
		echo "ssl.trusted_certs.container = JKS" >> $OUTPUT
	fi
}

usage() {
	echo "$0 -s -c <cert dir> -d <debug level> {dir|mrc|osd0..9} <testdir>"
	echo ""
	exit 1
}



DIR_PORT=32638
MRC_PORT=32636
OSD_START_PORT=32640

SSL_ENABLED=0
CERT_DIR=""
DEBUG=1

while getopts ":sc:d:" Option
# Initial declaration.
# a, b, c, d, e, f, and g are the options (flags) expected.
# The : after option 'e' shows it will have an argument passed with it.
do
  case $Option in
    s ) SSL_ENABLED=1
	;;
    c ) CERT_DIR=$OPTARG
	;;
    d ) DEBUG=$OPTARG
	;;
  esac
done
shift $(($OPTIND - 1))

if [ $# -ne 2 ]
then
	usage $0;
	exit 1;
fi

SERVICE=$1
TEST_DIR=$2

if [ $SERVICE == "dir" ]
then
	dir_config
elif [ $SERVICE == "mrc" ]
then
	mrc_config
elif [ ${SERVICE:0:3} == "osd" ]
then
	osd_config ${SERVICE:3:4}
else
	echo "unknown service type $SERVICE"
	exit 1
fi
