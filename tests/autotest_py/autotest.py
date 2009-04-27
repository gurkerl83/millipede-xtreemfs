import sys, os.path, signal, shutil, time, traceback, unittest
from errno import *
from optparse import OptionParser
from subprocess import Popen, call


# Constants
# Private key passphrases
CLIENT_PKCS12_PASSPHRASE = "passphrase"
DIR_PKCS12_PASSPHRASE = "passphrase" 
MRC_PKCS12_PASSPHRASE = "passphrase" 
OSD_PKCS12_PASSPHRASE = "passphrase" 
TRUSTED_CERTS_JKS_PASSPHRASE = "passphrase"

# Paths
MY_DIR_PATH = os.path.dirname( os.path.abspath( sys.modules[__name__].__file__ ) )
XTREEMFS_DIR_PATH = os.path.abspath( os.path.join( MY_DIR_PATH, "..", ".." ) )

CERT_DIR_PATH = os.path.abspath( os.path.join( MY_DIR_PATH, "..", "certs" ) )
CLIENT_PKCS12_FILE_PATH = os.path.join( CERT_DIR_PATH, "Client.p12" )
DIR_PKCS12_FILE_PATH = os.path.join( CERT_DIR_PATH, "DIR.p12" )
MRC_PKCS12_FILE_PATH = os.path.join( CERT_DIR_PATH, "MRC.p12" )
OSD_PKCS12_FILE_PATH = os.path.join( CERT_DIR_PATH, "OSD.p12" )
TRUSTED_CERTS_JKS_FILE_PATH = os.path.join( CERT_DIR_PATH, "trusted.jks" )

CLASSPATH = ( os.path.join( XTREEMFS_DIR_PATH, "servers", "dist", "XtreemFS.jar" ),
              os.path.join( XTREEMFS_DIR_PATH, "servers", "lib", "BabuDB-0.1.0-RC.jar" ) )

XTFS_MKVOL_FILE_PATH = os.path.join( XTREEMFS_DIR_PATH, "client", "bin", "xtfs_mkvol" + ( sys.platform.startswith( "win" ) and ".exe" or "" ) )
XTFS_MOUNT_FILE_PATH = os.path.join( XTREEMFS_DIR_PATH, "client", "bin", "xtfs_mount" + ( sys.platform.startswith( "win" ) and ".exe" or "" ) )

              
# Ports
DIR_HTTP_PORT = 30638
DIR_ONCRPC_PORT = 32638
MRC_HTTP_PORT = 30636 
MRC_ONCRPC_PORT = 32636
OSD_HTTP_PORT = 30640
OSD_ONCRPC_PORT = 32640

# Option defaults
DEBUG_LEVEL_DEFAULT = 0
NUM_OSDS_DEFAULT = 1
SSL_ENABLED_DEFAULT = False
STRIPE_SIZE_DEFAULT = 128
STRIPING_POLICY_TYPE_DEFAULT = "RAID0"


def check_environment():
    if os.environ.has_key( "JAVA_HOME" ):
#        JVERS=`$JAVA_HOME/bin/java -version 2>&1 | grep "java version" | \
#            cut -d " " -f 3`
#        perl -e " exit 1 if ($JVERS < \"1.6.0\");"
#        if [ $? -eq 1 ]; then
#            echo "ERROR: java version is $JVERS but required is >= 1.6.0"
#            exit 1
#        fi
#    fi
#    echo "java home             $JAVA_HOME"
        pass
    else:
        print "$JAVA_HOME not set, JDK/JRE 1.6 required"
        return False

    for file_path in ( XTFS_MOUNT_FILE_PATH, ) + CLASSPATH:
        if not os.path.exists( file_path ):
            print "ERROR: %(file_path)s does not exist." % locals()
            return False
    
    return True

    
def execute_tests( num_osds=NUM_OSDS_DEFAULT ):
    tests_dir_path = os.path.join( MY_DIR_PATH, "tests" )
    sys.path.append( tests_dir_path )
    original_cwd = os.getcwd()
    for file_name in os.listdir( tests_dir_path ):
        if file_name.endswith( ".py" ):
            test_module_name = os.path.splitext( file_name )[0]
            print "Trying to import test", test_module_name
            try:
                test_module = __import__( test_module_name )
            except ImportError:
                print "Could not import", test_module_name, "from tests"
                traceback.print_exc()
                continue

            if hasattr( test_module, "suite" ):
                suite = getattr( test_module, "suite" )
                for mount_point_dir_name in os.listdir( os.path.join( MY_DIR_PATH, "mnt" ) ):
                    mount_point_dir_path = os.path.join( MY_DIR_PATH, "mnt", mount_point_dir_name )
                    print "Running", test_module_name, "in", mount_point_dir_path
                    test_run_dir_path = os.path.join( mount_point_dir_path, test_module_name )
                    try: os.mkdir( test_run_dir_path )
                    except: pass                                         
                    os.chdir( test_run_dir_path )
                    unittest.TextTestRunner().run( suite )
                    os.chdir( mount_point_dir_path )
                    shutil.rmtree( test_run_dir_path )
            else:
                print "Test module", test_module_name, "does not have a suite global variable"

    os.chdir( original_cwd )                    


def generate_dir_config( debug_level=DEBUG_LEVEL_DEFAULT, ssl_enabled=SSL_ENABLED_DEFAULT ):
    locals().update( globals() )
    database_dir = os.path.join( "data", "dir" )
    open( os.path.join( "config", "dir.config" ), "w+" ).write( """\
uuid = test-env-DIR
debug_level = %(debug_level)s
listen.port = %(DIR_ONCRPC_PORT)u
http_port = %(DIR_HTTP_PORT)u
database.dir = %(database_dir)s
authentication_provider = org.xtreemfs.common.auth.NullAuthProvider
""" % locals() + _generate_ssl_config( ssl_enabled, DIR_PKCS12_FILE_PATH, DIR_PKCS12_PASSPHRASE ) )
 
def generate_mrc_config( debug_level=DEBUG_LEVEL_DEFAULT, ssl_enabled=SSL_ENABLED_DEFAULT ):
    locals().update( globals() )
    database_log = os.path.join( "data", "mrc" )
    database_dir = os.path.join( "data", "mrc" )
    open( os.path.join( "config", "mrc.config" ), "w+" ).write( """\
uuid = test-env-MRC
debug_level = %(debug_level)s
listen.port = %(MRC_ONCRPC_PORT)u
http_port = %(MRC_HTTP_PORT)u 
dir_service.port = %(DIR_ONCRPC_PORT)u
dir_service.host = localhost
database.log = %(database_log)s
database.dir = %(database_dir)s
osd_check_interval = 300
no_atime = true
no_fsync = true
local_clock_renewal = 50
remote_time_sync = 60000
database.checkpoint.interval = 1800000
database.checkpoint.idle_interval = 1000
database.checkpoint.logfile_size = 16384
authentication_provider = org.xtreemfs.common.auth.NullAuthProvider
capability_secret = testsecret
""" % locals() + _generate_ssl_config( ssl_enabled, MRC_PKCS12_FILE_PATH, MRC_PKCS12_PASSPHRASE ) )

def generate_osd_config( debug_level=DEBUG_LEVEL_DEFAULT, ssl_enabled=SSL_ENABLED_DEFAULT, osdnum=1 ):
    locals().update( globals() )
    listen_port = OSD_ONCRPC_PORT + int( osdnum ) - 1
    http_port = OSD_HTTP_PORT + int( osdnum ) - 1
    object_dir = os.path.join( "data", "osd" + str( int( osdnum ) - 1 ) )
    open( os.path.join( "config", "osd" + str( osdnum ) + ".config" ), "w+" ).write( """\
uuid = test-env-OSD%(osdnum)s
debug_level = %(debug_level)s
listen.port = %(listen_port)u
http_port = %(http_port)u
dir_service.host = localhost
dir_service.port = %(DIR_ONCRPC_PORT)u
object_dir=%(object_dir)s
local_clock_renewal = 50
remote_time_sync = 60000
report_free_space = true
checksums.enabled = false
capability_secret = testsecret
""" % locals() + _generate_ssl_config( ssl_enabled, OSD_PKCS12_FILE_PATH, OSD_PKCS12_PASSPHRASE ) )

def _generate_ssl_config( ssl_enabled, pkcs12_file_path, pkcs12_passphrase ):
    locals().update( globals() )
    if ssl_enabled:            
        return """\
ssl.enabled = true
ssl.service_creds = %(pkcs12_file_path)s
ssl.service_creds.pw = %(pks12_passphrase)s
ssl.service_creds.container = PKCS12
ssl.trusted_certs = %(TRUSTED_CERTS_JKS_FILE_PATH)s
ssl.trusted_certs.pw = %(TRUSTED_CERTS_JCS_PASSPHRASE)s
ssl.trusted_certs.container = JKS        
""" % locals()
    else:        
        return """\
ssl.enabled = false        
"""            
    

def start_environment( debug_level=DEBUG_LEVEL_DEFAULT, num_osds=NUM_OSDS_DEFAULT, ssl_enabled=SSL_ENABLED_DEFAULT, create_volumes=True, mount_volumes=True ):
    # prepare_test_directory   
    for subdir_name in ( "config", "data", "log", "mnt", "run" ):
        if not os.path.exists( subdir_name ):
            os.mkdir( subdir_name )
    
    generate_dir_config( debug_level, ssl_enabled )
    generate_mrc_config( debug_level, ssl_enabled )
    for osdnum in xrange( 1, num_osds+1 ):
        generate_osd_config( debug_level, ssl_enabled, osdnum )

    _start_service( "dir" )
    time.sleep( 0.5 )
    _start_service( "mrc" )    
    for osdnum in xrange( 1, num_osds+1 ):
        _start_service( "osd", str( osdnum ) )

    if ssl_enabled:
        client_ssl_args = ( "--pkcs12-file-path=%(CLIENT_PKCS12_FILE_PATH)s", "--pkcs12-passphrase=%(CLIENT_PKCS12_PASSPHRASE)s" )

    if create_volumes:
        for osdnum in xrange( 1, num_osds+1 ):
            xtfs_mkvol_args = [XTFS_MKVOL_FILE_PATH]
            xtfs_mkvol_args.extend( ( "-d", str( debug_level ) ) )
            if ssl_enabled: xtfs_mkvol_args.extend( client_ssl_args )                
            xtfs_mkvol_args.extend( ( "-p", str( STRIPING_POLICY_TYPE_DEFAULT ) ) )
            xtfs_mkvol_args.extend( ( "-s", str( STRIPE_SIZE_DEFAULT ) ) )
            xtfs_mkvol_args.extend( ( "-w", str( osdnum + 1 ) ) ) # Stripe width
            xtfs_mkvol_args.append( "localhost/test_%(osdnum)u" % locals() )
            xtfs_mkvol_args = " ".join( xtfs_mkvol_args )                            
            print "Creating volume test_%(osdnum)s with command line" % locals(), xtfs_mkvol_args
            retcode = call( xtfs_mkvol_args, shell=True )
            if retcode != 0 and retcode != EEXIST:
                print "FAILED: cannot create volume test_%(osdnum)u" % locals()
                stop_environment( num_osds )
                sys.exit( 1 )

    if mount_volumes:
        for osdnum in xrange( 1, num_osds+1 ):
            for direct_io in ( False, True ):
                mount_dir_name = str( osdnum ) + ( not direct_io and "_nondirect" or "" )
                mount_dir_path = os.path.join( "mnt", mount_dir_name ) 
                xtfs_mount_args = [XTFS_MOUNT_FILE_PATH]
                xtfs_mount_args.extend( ( "-d", str( debug_level ) ) )                
                xtfs_mount_args.append( "-f" ) # So we can redirect stdout and stderr
                if ssl_enabled: xtfs_mount_args.extend( client_ssl_args )
                if direct_io: xtfs_mount_args.extend( ( "-o", "direct_io" ) )
                DIR_ONCRPC_PORT = globals()["DIR_ONCRPC_PORT"]
                xtfs_mount_args.append( "localhost:%(DIR_ONCRPC_PORT)s/test_%(osdnum)u" % locals() )                
                xtfs_mount_args.append( mount_dir_path )
                xtfs_mount_args = " ".join( xtfs_mount_args )
                print "Mounting volume test_%(osdnum)s with command line" % locals(), xtfs_mount_args
                if not os.path.exists( mount_dir_path ): os.mkdir( mount_dir_path )                
                log_file_path = os.path.join( "log", "client" + mount_dir_name + ".log" )
                stderr = stdout = open( log_file_path, "a" )
                xtfs_mount_process = Popen( xtfs_mount_args, shell=True, stdout=stdout, stderr=stderr )
            

def _start_service( service_name, service_num="" ):        
    args = [os.path.join( os.environ["JAVA_HOME"], "bin", "java" )] 
    args.append( "-ea" ) # Enable assertions
    args.extend( ( "-cp", "" + ( sys.platform.startswith( "win" ) and ";".join( CLASSPATH ) or ":".join( CLASSPATH ) ) + "", ) )
    args.append( "org.xtreemfs." + service_name + "." + service_name.upper() ) # Class name
    args.append( os.path.join( "config", service_name + service_num + ".config" ) ) # Configuration file
    args = " ".join( args )
    log_file_path = os.path.join( "log", service_name + service_num + ".log" )
    stderr = stdout = open( log_file_path, "a" )
    print "Starting", service_name.upper() + service_num, "service with command line", args, "and redirecting output to", log_file_path
    p = Popen( args, shell=True, stdout=stdout, stderr=stderr )
    open( os.path.join( "run", service_name + service_num + ".run" ), "w+" ).write( str( p.pid ) )


def stop_environment( num_osds=NUM_OSDS_DEFAULT ):
    _stop_service( "dir" )
    _stop_service( "mrc" )
    for osdnum in xrange( 1, num_osds+1 ):
        _stop_service( "osd", str( osdnum ) )

    mnt_dir_path = os.path.abspath( "mnt" )
    for mounts_line in open( "/proc/mounts" ).readlines():
       mounts_line_parts = mounts_line.split()
       device = mounts_line_parts[0]
       mount_point_dir_path = mounts_line_parts[1]
       if device == "/dev/fuse" and mount_point_dir_path.startswith( mnt_dir_path ):
           fusermount_args = " ".join( ["fusermount", "-u", mount_point_dir_path] )
           print "Unmounting volume with", fusermount_args
           call( fusermount_args, shell=True )

def _stop_service( service_name, service_num="" ):
    run_file_path = os.path.join( "run", service_name + service_num + ".run" )
    if os.path.exists( run_file_path ):
        service_pid = int( open( run_file_path ).read().strip() )
        print "Stopping", service_name.upper() + service_num, "service with PID", service_pid
        if sys.platform.startswith( "win" ):
            call( "TASKKILL /PID %(service_pid)u /F /T" % locals() )
        else:
            try: 
               os.kill( service_pid, signal.SIGTERM )
            except:
               traceback.print_exc()
        os.unlink( run_file_path )
    else:
        print "ERROR:", run_file_path, "does not exist,", service_name.upper(), "service is not running."
        

if __name__ == "__main__":
    if not check_environment():
        sys.exit( 1 )

    
    option_parser = OptionParser()
    option_parser.add_option( "-d", action="store", dest="debug_level", default=DEBUG_LEVEL_DEFAULT )
    option_parser.add_option( "--execute_tests", action="store_true", dest="execute_tests" )
    option_parser.add_option( "-m", action="store_true", dest="disable_create_volumes" )
    option_parser.add_option( "-n", action="store_true", dest="disable_mount_volumes" )
    option_parser.add_option( "-o", action="store", dest="num_osds", type="int", default=NUM_OSDS_DEFAULT )
    option_parser.add_option( "--start", action="store_true", dest="start_environment" )
    option_parser.add_option( "--start_environment", action="store_true", dest="start_environment" )
    option_parser.add_option( "--stop", action="store_true", dest="stop_environment" )
    option_parser.add_option( "--stop_environment", action="store_true", dest="stop_environment" )
    option_parser.add_option( "-s", action="store_true", dest="ssl_enabled", default=SSL_ENABLED_DEFAULT )
    option_parser.add_option( "--test", action="store_true", dest="execute_tests" )
    options, ignore = option_parser.parse_args()

        
    if options.execute_tests:
        execute_tests()
    elif options.stop_environment:
        stop_environment()
    else:
        stop_environment()
        start_environment( debug_level=options.debug_level, ssl_enabled=options.ssl_enabled, num_osds=options.num_osds, create_volumes=not options.disable_create_volumes, mount_volumes=not options.disable_mount_volumes )
        if not options.start_environment: # i.e. no options were specified
            try:
                execute_tests( num_osds=options.num_osds )
            except:
                traceback.print_exc()
            stop_environment()
    
        