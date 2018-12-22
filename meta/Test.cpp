#ifdef TEST

#include <cpp/Program.h>
#include <cpp/io/StandardOutput.h>
#include <cpp/meta/Unittest.h>

int main(int argc, char const * argv[])
{
    try
    {
        cpp::Program::Standard program{ argc, argv };

        cpp::Logger::addStdout( cpp::LogLevel::Alert );
        cpp::Logger::addDebug( cpp::LogLevel::Debug );
        cpp::Logger::addFile( ".unittest", "unittest", "log.txt", cpp::LogLevel::Debug );

        cpp::log( "Starting tests..." );

        int iters = 1;
        for ( int i = 0; i < iters; i++ )
        {
            int result = UnitTest::RunAllTests( ".unittest/unittest.xml" );
            if ( result != 0 )
            {
                return result;
            }
        }

        cpp::log( "Finished tests..." );
    }
    catch ( std::exception & e )
    {
        cpp::print( "error: %\n", e.what( ) );
    }

	return 0;
}


#endif
