#pragma once

#include <cpp/String.h>

namespace cpp
{

    inline uint32_t atox( const Memory & src )
        { return strtoul( src.c_str( ), 0, 16 ); }

    String toURI( const Memory & src );
    String fromURI( const Memory & src );

    String toHex( const Memory & src, bool caseUpper = false, bool reverse = false );
    String fromHex( const Memory & src, bool reverse = false );

    String toBase64( Memory data, bool usePadding = false );
    String fromBase64( Memory data );

    template<typename T> String valueToHex( T value, bool caseUpper = false )
        { return cpp::toHex( Memory::ofValue(value), caseUpper, true ); }
    template<typename T> T valueFromHex( const Memory & src )
    {
        T result = 0;
        String decodedValue = fromHex( src );
        check<std::invalid_argument>( decodedValue.length( ) == sizeof( T ),
            "invalid input for decode" );
        Memory::copy( Memory::ofValue( result ), decodedValue );
        return result;
    }

}