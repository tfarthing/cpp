#ifndef TEST

#include "DataMap.h"

namespace cpp
{

    

	/*
    void encodeBinary( ReadBuffer & buffer, const cpp::DataMap & value, ByteOrder byteOrder )
    {
        buffer.putBinary( (uint32_t)value.size( ), byteOrder );
        for ( auto & item : value )
        {
            buffer.putBinary( item.first, byteOrder );
            buffer.putBinary( item.second, byteOrder );
        }
    }

    void decodeBinary( ReadBuffer & buffer, cpp::DataMap & value, ByteOrder byteOrder )
    {
        uint32_t size;
        buffer.getBinary( size, byteOrder );

        value.clear( );
        for ( uint32_t i = 0; i < size; i++ )
        {
            cpp::String k;
            cpp::String v;
            buffer.getBinary( k, byteOrder );
            buffer.getBinary( v, byteOrder );
            value.insert_or_assign( k, v );
        }
    }
	*/

}

#else

#include <cpp/meta/Unittest.h>

#include "DataMap.h"

SUITE( DataMap )
{
    
    cpp::String returnString( )
    {
        return "string";
    }

    cpp::DataMap createMap( cpp::DataMap input )
    {
        return input;
    }

    TEST( test1 )
    {
        cpp::StringMap dataMap = createMap( 
            {
                { "key1", cpp::String{ "value1" } },
                { "key2", returnString( ) }
            });

        CHECK( dataMap["key1"] == "value1" );
        CHECK( dataMap["key2"] == "string" );
    }
}

#endif
