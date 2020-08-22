#ifndef TEST

#include "Float.h"
#include "String.h"

namespace cpp
{

	float64 Float::parse( Memory text )
	{
        float64 result = strtod( text.data( ), nullptr );
		check<Exception>( errno != ERANGE, "range error while parsing float" );
		return result;
	}


	//  specialized form of floating-point to ASCII
	//  * precision of fractional digits are specified
	//  * fracitional part is rounded to the specified precision
	//  * trailing zeroes are trimmed
	//  * integer is output if fractional part is zero
	std::string Float::toString( float64 value, int fdigits )
	{
        String format = String::printf( "%%.%df", fdigits );
		String result = String::printf( format.data.data(), value );
        result.trimBack( "0" );
        result.trimBack( "." );
		return std::move(result.data);
	}

}

#else

#include <cpp/meta/Test.h>

#include "Float.h"

TEST_CASE( "Float" )
{
    using namespace cpp;

    SECTION( "toString" )
    {
        CHECK( Float::toString( 0.100, 2 ) == "0.1" );
        CHECK( Float::toString( 0.106, 2 ) == "0.11" );
        CHECK( Float::toString( 100.001, 2 ) == "100" );
        CHECK( Float::toString( 100.0011, 3 ) == "100.001" );
        CHECK( Float::toString( 100.1011, 1 ) == "100.1" );
    }
}

#endif
