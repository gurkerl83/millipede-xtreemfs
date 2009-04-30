import unittest, os.path, sys, subprocess


# Constants
MY_DIR_PATH = os.path.dirname( os.path.abspath( sys.modules[__name__].__file__ ) )
MARKED_BLOCK_PL_FILE_PATH = os.path.join( MY_DIR_PATH, "marked_block.pl" )
                               

class ErichsDataIntegrityTest(unittest.TestCase):
    def runTest( self ):
        p = subprocess.Popen( MARKED_BLOCK_PL_FILE_PATH + " --start=1 --nfiles=20 --size=1 --group=10 --base=.", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
        retcode = p.wait()
        if retcode != 0:
            print "Unexpected return code from marked_block.pl: " + str( retcode )
            print "Output:"
            print p.stdout.read()
            self.assertEqual( retcode, 0 )

                
suite = unittest.TestSuite()
suite.addTest( ErichsDataIntegrityTest() )
        

if __name__ == "__main__":
    unittest.TextTestRunner( verbosity=2 ).run( suite )
    