#include "Integer.h"
#include "String.h"
#include "../process/Exception.h"

namespace cpp
{

	int64_t Integer::parse( Memory text, int radix, bool checkEnding )
	{
		char * end = nullptr;
		int64_t result = strtoll( text.data( ), &end, radix );
		check<DecodeException>( errno != ERANGE, "Integer::parse() : range error while parsing integer" );
		if ( checkEnding )
			{ check<DecodeException>( end == text.end(), "Integer::parse() : parse ended before the buffer's end" ); }
		return result;
	}


	uint64_t Integer::parseUnsigned( Memory text, int radix, bool checkEnding )
	{
		char * end = nullptr;
		uint64_t result = strtoull( text.data( ), &end, radix );
		check<DecodeException>( errno != ERANGE, "Integer::parseUnsigned() : range error while parsing integer" );
		if ( checkEnding )
			{ check<DecodeException>( end == text.end(), "Integer::parseUnsigned() : parse ended before the buffer's end" ); }
		return result;
	}


	std::string Integer::toHex( uint64_t value, int width, bool upper, bool zeroed, bool prefix )
	{
		String fmt = "%";
		if ( prefix )
			{ fmt += "#"; }
		if ( zeroed )
			{ fmt += "0"; }
		if ( width > 0 )
			{ fmt += std::to_string( width ); }
		fmt += upper ? "llX" : "llx";

		return String::printf( fmt.c_str( ), value );
	}


	std::string Integer::toDecimal( int64_t value, int width, bool zeroed, bool sign )
	{
		String fmt = "%";
		if ( sign )
			{ fmt += "+"; }
		if ( zeroed )
			{ fmt += "0"; }
		if ( width > 0 )
			{ fmt += std::to_string( width ); }
		fmt += "lli";

		return String::printf( fmt.c_str( ), value );
	}


	std::string Integer::toDecimal( uint64_t value, int width, bool zeroed, bool sign )
	{
		String fmt = "%";
		if ( sign )
			{ fmt += "+"; }
		if ( zeroed )
			{ fmt += "0"; }
		if ( width > 0 )
			{ fmt += std::to_string( width ); }
		fmt += "llu";

		return String::printf( fmt.c_str( ), value );
	}
}