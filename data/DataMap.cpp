#ifndef TEST

#include <cpp/util/BitData.h>
#include "DataMap.h"

namespace cpp
{

    uint64_t RecordMap::getUint( Memory key ) const
        { return BitData::decodeUint( get( key ) ); }
    uint64_t RecordMap::getHex( Memory key ) const
        { return BitData::decodeHex( get( key ) ); }
    int64_t RecordMap::getInt( Memory key ) const
        { return BitData::decodeInt( get( key ) ); }
    f64_t RecordMap::getFloat( Memory key ) const
        { return BitData::decodeF64( get( key ) ); }
    bool RecordMap::getBool( Memory key ) const
        { return BitData::decodeBool( get( key ) ); }
    DateTime RecordMap::getTime( Memory key ) const
        { return BitData::decodeTime( get( key ) ); }

    void RecordMap::setUint( Memory key, uint64_t value )
        { set( key, BitData::encodeUint( value ) ); }
    void RecordMap::setHex( Memory key, uint64_t value )
        { set( key, BitData::encodeHex( value ) ); }
    void RecordMap::setInt( Memory key, int64_t value )
        { set( key, BitData::encodeInt( value ) ); }
    void RecordMap::setFloat( Memory key, f64_t value )
        { set( key, BitData::encodeF64( value ) ); }
    void RecordMap::setBool( Memory key, bool value )
        { set( key, BitData::encodeBool( value ) ); }
    void RecordMap::setTime( Memory key, DateTime value )
        { set( key, BitData::encodeTime( value ) ); }
    
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
