#pragma once

#include <map>
#include <functional>

#include <cpp/chrono/DateTime.h>
#include <cpp/String.h>
#include <cpp/util/DataBuffer.h>
#include <cpp/util/DataMap.h>

//  Binary In Text File
class BitData
{
public:
    struct DecodedValue
    {
        cpp::Memory value;
        cpp::String buffer;
    };
    struct DecodedRecord
    {
        cpp::Memory key;
        std::map<cpp::Memory, DecodedValue> data;
    };
    struct DecodedKeyValue
    {
        cpp::Memory key;
        DecodedValue value;
    };
    struct Record
    {
        cpp::Memory key;
        cpp::StringMap data;
    };

    static cpp::String encodeKeyValue( cpp::Memory dataKey, cpp::Memory dataValue, bool scanValue = true );
    static DecodedKeyValue decodeKeyValue( cpp::ReadBuffer & buffer );

    static cpp::String encodeData( const cpp::DataMap & data, bool scanValue = true );

    static cpp::String encode( cpp::Memory id, const cpp::DataMap & data, bool scanValue = true );
    static cpp::String encode( const Record & record, bool scanValue = true );
    static DecodedRecord decode( cpp::Memory text );

    // utility encode/decode functions
    static cpp::String encodeInt( int64_t value );
    static cpp::String encodeHex( uint64_t value );
    static cpp::String encodeUint( uint64_t value );
    static cpp::String encodeF64( f64_t value );
    static cpp::String encodeBool( bool value );
    static cpp::String encodeTime( cpp::DateTime value );
    template<class T> static cpp::String encodeList( const T & value );

    static int64_t decodeInt( const cpp::String & text );
    static uint64_t decodeHex( const cpp::String & text );
    static uint64_t decodeUint( const cpp::String & text );
    static f64_t decodeF64( const cpp::String & text );
    static bool decodeBool( const cpp::String & text );
    static cpp::DateTime decodeTime( const cpp::String & text );
    static cpp::Memory::Array decodeList( cpp::Memory text );
};

template<class T> cpp::String BitData::encodeList( const T & value )
{
    cpp::String result;
    for ( auto & item : value )
    {
        if ( !result.empty( ) )
            { result += ","; }
        result += item;
    }
    return result;
}

inline cpp::Memory::Array BitData::decodeList( cpp::Memory text )
{
    return text.split( "," );
}