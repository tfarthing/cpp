 #pragma once

#include <stdint.h>

#include <cpp/data/Memory.h>
#include <cpp/process/Exception.h>

namespace cpp
{

	struct Integer
	{
		static int64_t parse( Memory text, int radix = 10 );
		static uint64_t parseUnsigned( Memory text, int radix = 10 );

		static std::string toHex( uint64_t value, int width = 8, int precision = 0, bool upper = false, bool zeroed = true, bool prefix = true );
		static std::string toDecimal( int64_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false );
		static std::string toDecimal( uint64_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false );
    
        static std::string toDecimal( int32_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (int64_t)value, width, precision, zeroed, sign); }
        static std::string toDecimal( int16_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (int64_t)value, width, precision, zeroed, sign); }
        static std::string toDecimal( int8_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (int64_t)value, width, precision, zeroed, sign); }

        static std::string toDecimal( uint32_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (uint64_t)value, width, precision, zeroed, sign); }
        static std::string toDecimal( uint16_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (uint64_t)value, width, precision, zeroed, sign); }
        static std::string toDecimal( uint8_t value, int width = 0, int precision = 0, bool zeroed = false, bool sign = false )
            { return toDecimal( (uint64_t)value, width, precision, zeroed, sign); }

    };

}
