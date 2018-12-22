#include <cpp/data/Integer.h>
#include <cpp/data/String.h>

namespace cpp
{

	static int64_t parse( Memory text, int radix = 10 )
	{
		int64_t result = strtoll( text.data( ), nullptr, radix );
		check<Exception>( errno != ERANGE, "range error while parsing integer" );
		return result;
	}

	static uint64_t parseUnsigned( Memory text, int radix = 10 )
	{
		uint64_t result = strtoull( text.data( ), nullptr, radix );
		check<Exception>( errno != ERANGE, "range error while parsing integer" );
		return result;
	}

	static std::string toHex( uint64_t value, int width = 8, int precision = 0, bool upper = false, bool zeroed = true, bool prefix = true )
	{
		String fmt = "%";
		if ( prefix )
		{
			fmt += "#";
		}
		if ( zeroed )
		{
			fmt += "0";
		}
		if ( width > 0 )
		{
			fmt += Integer::toDecimal( (int64_t)width );
		}
		if ( precision > 0 )
		{
			fmt += "." + Integer::toDecimal( (int64_t)precision );
		}
		fmt += upper ? "llX" : "llx";

		return String::printf( fmt.c_str( ), value );
	}

	static std::string toDecimal( int64_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
	{
		String fmt = "%";
		if ( sign )
		{
			fmt += "+";
		}
		if ( zeroed )
		{
			fmt += "0";
		}
		if ( width > 0 )
		{
			fmt += Integer::toDecimal( width );
		}
		if ( precision > 0 )
		{
			fmt += "." + Integer::toDecimal( precision );
		}
		fmt += "lli";

		return String::printf( fmt.c_str( ), value );
	}

	static std::string toDecimal( uint64_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
	{
		String fmt = "%";
		if ( sign )
		{
			fmt += "+";
		}
		if ( zeroed )
		{
			fmt += "0";
		}
		if ( width > 0 )
		{
			fmt += Integer::toDecimal( (int64_t)width );
		}
		if ( precision > 0 )
		{
			fmt += "." + Integer::toDecimal( (int64_t)precision );
		}
		fmt += "llu";

		return String::printf( fmt.c_str( ), value );
	}
}