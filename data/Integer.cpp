#ifndef TEST

#include "Integer.h"
#include "String.h"
#include "../process/Exception.h"

namespace cpp
{

	int64 Integer::parse( Memory text, int radix, bool checkEnding )
	{
		char * end = nullptr;
		int64 result = strtoll( text.data( ), &end, radix );
		check<DecodeException>( errno != ERANGE, "Integer::parse() : range error while parsing integer" );
		if ( checkEnding )
			{ check<DecodeException>( end == text.end(), "Integer::parse() : parse ended before the buffer's end" ); }
		return result;
	}


	uint64 Integer::parseUnsigned( Memory text, int radix, bool checkEnding )
	{
		char * end = nullptr;
		uint64 result = strtoull( text.data( ), &end, radix );
		check<DecodeException>( errno != ERANGE, "Integer::parseUnsigned() : range error while parsing integer" );
		if ( checkEnding )
			{ check<DecodeException>( end == text.end(), "Integer::parseUnsigned() : parse ended before the buffer's end" ); }
		return result;
	}


	std::string Integer::toHex( uint64 value, int width, bool upper, bool zeroed, bool prefix )
	{
		String fmt = "%";
		if ( prefix )
			{ fmt += "#"; }
		if ( zeroed )
			{ fmt += "0"; }
		if ( width > 0 )
			{ fmt += std::to_string( width ); }
		fmt += upper ? "llX" : "llx";

		return String::printf( fmt.begin( ), value );
	}


	std::string Integer::toDecimal( int64 value, int width, bool zeroed, bool sign )
	{
		String fmt = "%";
		if ( sign )
			{ fmt += "+"; }
		if ( zeroed )
			{ fmt += "0"; }
		if ( width > 0 )
			{ fmt += std::to_string( width ); }
		fmt += "lli";

		return String::printf( fmt.begin( ), value );
	}


	std::string Integer::toDecimal( uint64 value, int width, bool zeroed, bool sign )
	{
		String fmt = "%";
		if ( sign )
			{ fmt += "+"; }
		if ( zeroed )
			{ fmt += "0"; }
		if ( width > 0 )
			{ fmt += std::to_string( width ); }
		fmt += "llu";

		return String::printf( fmt.begin( ), value );
	}
}

#else

#include <cpp/meta/Test.h>

#include "Integer.h"

TEST_CASE( "Integer" )
{
    using namespace cpp;

    SECTION( "to" )
    {
        CHECK( Integer::to<uint8>( 1 ) == 1 );
        //CHECK( Integer::to<uint8>( -2 ) == -2 );
        CHECK( Integer::to<uint8>( 255 ) == 255 );
        //CHECK( Integer::to<uint8>( 256 ) == 256 );
        CHECK( Integer::to<int8>( -128 ) == -128 );
        //CHECK( Integer::to<int8>( -129 ) == -129 );
    }
}

#endif
