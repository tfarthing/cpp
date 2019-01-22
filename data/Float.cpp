#include "Float.h"
#include "String.h"

namespace cpp
{

	f64_t Float::parse( Memory text )
	{
		f64_t result = strtod( text.data( ), nullptr );
		check<Exception>( errno != ERANGE, "range error while parsing float" );
		return result;
	}


	//  specialized form of floating-point to ASCII
	//  * precision of fractional digits are specified
	//  * fracitional part is rounded to the specified precision
	//  * trailing zeroes are trimmed
	//  * integer is output if fractional part is zero
	std::string Float::toString( f64_t value, int fdigits )
	{
		String result;

		value = roundTo( value, fdigits );

		int64_t ivalue = (int64_t)value;
		if ( ivalue == 0 && value < 0.0 )
		{
			result += "-";
		}
		result += String::printf( "%lld", ivalue );

		int mod = (int)std::pow( 10, fdigits );
		int64_t fvalue = (int64_t)( value * mod ) % mod;
		if ( fvalue != 0 )
		{
			result += ".";
			if ( fvalue < 0 )
			{
				fvalue *= -1;
			}
			//  trim trailing zeros
			while ( fvalue % 10 == 0 )
			{
				fvalue /= 10; mod /= 10;
			}
			//  include leading zeroes
			while ( ( mod /= 10 ) > 0 )
			{
				result += (char)( '0' + ( fvalue / mod % 10 ) );
			}
		}
		return result;
	}

}