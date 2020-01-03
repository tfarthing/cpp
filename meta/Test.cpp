#ifdef TEST

#define CATCH_CONFIG_RUNNER
#include "Test.h"

#include "../../cpp/process/Program.h"
#include "../../cpp/util/Log.h"


int main(int argc, char const * argv[])
{
	cpp::Program program{ argc, argv };
	program.logger( ).setDebug( cpp::LogLevel::Debug );

	try
    {
		int result = Catch::Session( ).run( argc, argv );

		return result;
	}
    catch ( std::exception & e )
    {
        //cpp::print( "error: %\n", e.what( ) );
		return -1;
	}
}


#endif
