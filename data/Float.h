#pragma once

#include "Integer.h"
#include "../process/Exception.h"

namespace cpp
{

	typedef float f32_t;
	typedef double f64_t;

	struct Float
	{

		static f64_t parse( Memory text );

		//  specialized form of floating-point to ASCII
		//  * precision of fractional digits are specified
		//  * fracitional part is rounded to the specified precision
		//  * trailing zeroes are trimmed
		//  * integer is output if fractional part is zero
		static std::string toString( f64_t value, int fdigits );

		static double roundTo( f64_t value, int fdigits );

		static f32_t fromBits( uint32_t bits );
		static f64_t fromBits( uint64_t bits );

		static uint32_t toBits( f32_t value );
		static uint64_t toBits( f64_t value );

	};

	inline double Float::roundTo( f64_t value, int fdigits )
	{
		double factor = std::pow( 10, fdigits );
		return std::round( value * factor ) / factor;
	}

	inline f32_t Float::fromBits( uint32_t bits )
		{ float value; memcpy( &value, &bits, sizeof( value ) ); return value; }

	inline f64_t Float::fromBits( uint64_t bits )
		{ double value; memcpy( &value, &bits, sizeof( value ) ); return value; }

	inline uint32_t Float::toBits( f32_t value )
		{ uint32_t bits; memcpy( &bits, &value, sizeof( bits ) ); return bits; }

	inline uint64_t toBits( f64_t value )
		{ uint64_t bits; memcpy( &bits, &value, sizeof( bits ) ); return bits; }

}