#pragma once

#include "../../cpp/data/Primitive.h"
#include "../../cpp/data/Memory.h"
#include "../process/Exception.h"

namespace cpp
{

	struct Float
	{

		static float64 parse( Memory text );

		//  specialized form of floating-point to ASCII
		//  * precision of fractional digits are specified
		//  * fracitional part is rounded to the specified precision
		//  * trailing zeroes are trimmed
		//  * integer is output if fractional part is zero
		static std::string toString( float64 value, int fdigits );

		static double roundTo( float64 value, int fdigits );

		static float32 fromBits( uint32 bits );
		static float64 fromBits( uint64 bits );

		static uint32 toBits( float32 value );
		static uint64 toBits( float64 value );

	};

	inline double Float::roundTo( float64 value, int fdigits )
	{
		double factor = std::pow( 10, fdigits );
		return std::round( value * factor ) / factor;
	}

	inline float32 Float::fromBits( uint32 bits )
		{ float value; memcpy( &value, &bits, sizeof( value ) ); return value; }

	inline float64 Float::fromBits( uint64 bits )
		{ double value; memcpy( &value, &bits, sizeof( value ) ); return value; }

	inline uint32 Float::toBits( float32 value )
		{ uint32 bits; memcpy( &bits, &value, sizeof( bits ) ); return bits; }

	inline uint64 toBits( float64 value )
		{ uint64 bits; memcpy( &bits, &value, sizeof( bits ) ); return bits; }

}