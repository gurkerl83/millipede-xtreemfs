import unittest, subprocess, sys, os


class iozoneDiagnosticTest(unittest.TestCase):
    def __init__( self, stdout=subprocess.PIPE, stderr=subprocess.STDOUT ):
        unittest.TestCase.__init__( self )
        self.stdout = stdout
        self.stderr = stderr
        
    def runTest( self ):
        if "nondirect" in os.getcwd():
            pass
        else:            
            args = "iozone -a -+d"
            p = subprocess.Popen( args, shell=True, stdout=self.stdout, stderr=self.stderr )
            retcode = p.wait()
            if retcode != 0:
                print >>self.stderr, "Unexpected return code from iozone:", retcode
                print >>self.stderr, "Output:"
                print >>self.stderr, p.stdout.read()
                self.fail()

            
def createTestSuite( *args, **kwds ): 
    if not sys.platform.startswith( "win" ):
        return unittest.TestSuite( [iozoneDiagnosticTest( *args, **kwds )] )
        

if __name__ == "__main__":
    if not sys.platform.startswith( "win" ):
        unittest.TextTestRunner( verbosity=2 ).run( createTestSuite() )
    else:
        print sys.modules[__name__].__file__.split( os.sep )[-1], "not supported on Windows"
    
