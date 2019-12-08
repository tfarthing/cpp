#ifdef TEST

#define CATCH_CONFIG_RUNNER
#include "Test.h"

#include "../../cpp/process/Program.h"


int main(int argc, char const * argv[])
{
	cpp::Program program{ argc, argv };
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
