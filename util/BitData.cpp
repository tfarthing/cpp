#ifndef TEST

#include <optional>

#include <cpp/Integer.h>
#include <cpp/util/encoding.h>
#include <cpp/Exception.h>
#include <cpp/chrono/Date.h>

#include "BitData.h"


/*
='value'
key='value'
 : key = 'value'
record:key='value'
record:key='value with escaped quote\' and newline\n'
record:key=(5)'value'
record:key1=(5)'value' key2='value'
record1:key='value' record2:key1='value' key2=(5)'value'
record : null
record : key=null

special parse characters are :='\n 
other special character is .

*/

void setValue(cpp::StringMap & data, cpp::Memory record, cpp::Memory key, cpp::Memory value)
{
    if (key)
    {
        data.set(record + "." + key, value);
    }
    else
    {
        data.set(record, value);
    }
}

void removeValue(cpp::StringMap & data, cpp::Memory record, cpp::Memory key)
{
    if (key)
    {
        data.erase(record + "." + key);
    }
    else
    {
        data.erase(record);
    }
}

void removeRecord(cpp::StringMap & data, cpp::Memory record)
{
    auto begin = data.lower_bound(record);
    if (begin == data.end())
        { return; }

    cpp::Memory parent = begin->first;
    if ( parent.substr( 0, record.length() ) == record && ( parent.length() == record.length() || parent[record.length()] == '.' ) )
        { return; }

    auto end = begin++;
    while (end != data.end())
    {
        cpp::Memory child = end->first;
        if (child.substr(0, record.length()) == record && child[record.length()] == '.')
            { break; }
        end++;
    }

    data.erase(begin, end);
}

std::optional<cpp::StringMap> BitDecode(cpp::ReadBuffer & buffer)
{
    enum class State 
        { BOL, PreToken, Token, PreValue, ValueSpec, Value, PostValue, Comment, Error, EOL};
    enum class Error 
        { Null, ExpectedKey, ExpectedAssignment, ExpectedValue, ExpectedValueOrValueSpec, ExpectedValueSpec, InvalidValueSpec, ExpectedValueDelimiter, ExpectedTokenDelimiter };

    std::function<cpp::Memory(Error)> toString = [](Error err) -> cpp::Memory
    {
        switch (err)
        {
        case Error::ExpectedKey:
            return "ExpectedKey";
        case Error::ExpectedAssignment:
            return "ExpectedAssignment";
        case Error::ExpectedValue:
            return "ExpectedValue";
        case Error::ExpectedValueOrValueSpec:
            return "ExpectedValueOrValueSpec";
        case Error::ExpectedValueSpec:
            return "ExpectedValueSpec";
        case Error::InvalidValueSpec:
            return "InvalidValueSpec";
        case Error::ExpectedValueDelimiter:
            return "ExpectedValueDelimiter";
        case Error::ExpectedTokenDelimiter:
            return "ExpectedTokenDelimiter";
        case Error::Null:
        default:
            return "Null";
        }
    };


    cpp::StringMap result;
    State state = State::BOL;
        
    size_t tokenPos = 0;
    bool escaped = false;
    cpp::String value;
    cpp::Memory valueSpec;
    cpp::Memory key;
    cpp::Memory record;
    size_t errorPos = 0;
    Error error = Error::Null;

    auto text = buffer.getable();
    size_t pos = 0;
    while (state != State::EOL)
    {
        uint8_t byte = buffer.getable().get(pos);
        
        switch (state)
        {
        case State::BOL:
        case State::PreToken:
            switch (byte)
            {
            case ' ':
            case '\t':
                break;
            case '\n':
            case '#':
                state = (byte == '\n') 
                    ? State::EOL 
                    : State::Comment;
                break;
            case ':':
                record = cpp::Memory::Null;
                state = State::PreToken;
                break;
            case '=':
                key = cpp::Memory::Null;
                state = State::PreValue;
                break;
            case 'n':
                if (buffer.getable().substr(pos, 4) == "null")
                {
                    pos += 4;
                    state = State::PostValue;
                    // todo assign record = null
                    removeRecord(result, record);

                    continue;
                }
                else
                {
                    tokenPos = pos;
                    state = State::Token;
                }
                break;
            default:
                tokenPos = pos;
                state = State::Token;
                break;
            }
            break;

        case State::PreValue:
            switch (byte)
            {
            case ' ':
            case '\t':
                break;
            case '(':
                if (valueSpec)
                {
                    errorPos = pos;
                    error = Error::ExpectedValue;
                    state = State::Error;
                }
                else
                {
                    tokenPos = pos;
                    state = State::ValueSpec;
                }
                break;
            case 'n':
                if (buffer.getable().substr(pos, 4) == "null")
                {
                    value.clear();
                    pos += 4;
                    state = State::PostValue;

                    // todo assign value = null
                    removeValue(result, record, key);

                    continue;
                }
                else
                {
                    errorPos = pos;
                    error = Error::ExpectedValueOrValueSpec;
                    state = State::Error;
                }
                break;
            case '\'':
                if (valueSpec)
                {
                    uint64_t len = cpp::Integer::parseUnsigned(valueSpec);
                    cpp::check<cpp::DecodeException>(pos + len + 2 > buffer.size(), "value spec size greater than buffer size");

                    if (buffer.getable().length() < pos + len + 2)
                        { return std::nullopt; }

                    if (buffer.getable().get(pos + len) != '\'')
                    {
                        errorPos = pos + len;
                        error = Error::ExpectedValueDelimiter;
                        state = State::Error;
                    }
                    else
                    {
                        // todo assign value
                        setValue(result, record, key, buffer.getable().substr(pos, len));
                        value.clear();
                        
                        pos += len;
                        state = State::PostValue;

                        continue;
                    }
                }
                else
                {
                    tokenPos = pos + 1;
                    state = State::Value;
                }

                break;
            default:
                errorPos = pos;
                error = Error::ExpectedValueOrValueSpec;
                state = (byte == '\n') 
                    ? State::EOL 
                    : State::Error;
                break;
            }
            break;

        case State::PostValue:
            switch (byte)
            {
            case '\n':
            case '#':
                state = (byte == '\n')
                    ? State::EOL
                    : State::Comment;
                break;
            case ' ':
            case '\t':
                state = State::PreToken;
                break;
            default:
                errorPos = pos;
                error = Error::ExpectedTokenDelimiter;
                state = State::Error;
                break;
            }
            break;

        case State::Token:
            switch (byte)
            {
            case '\n':
            case '#':
                errorPos = pos;
                error = Error::ExpectedAssignment;
                state = (byte == '\n') 
                    ? State::EOL 
                    : State::Error;
                break;
            case ':':
                record = text.substr(tokenPos, pos - tokenPos).trim();
                tokenPos = 0;
                state = State::PreToken;
                break;
            case '=':
                key = text.substr(tokenPos, pos - tokenPos).trim();
                state = State::PreValue;
                break;
            default:
                break;
            }
            break;

        case State::ValueSpec:
            switch (byte)
            {
            case '\n':
            case '#':
                errorPos = pos;
                error = Error::ExpectedValueSpec;
                state = (byte == '\n')
                    ? State::EOL
                    : State::Error;
                break;
            case ')':
                valueSpec = text.substr(tokenPos, pos - tokenPos).trim();
                tokenPos = 0;
                state = State::PreValue;
                break;
            default:
                if ((byte < '0' || byte > '9') && (byte < 'a' || byte > 'z'))
                {
                    errorPos = pos;
                    error = Error::InvalidValueSpec;
                    state = State::Error;
                }
                break;
            }
            break;

        case State::Value:
            switch (byte)
            {
            case '\n':
                errorPos = pos;
                error = Error::ExpectedValueDelimiter;
                state = State::EOL;
                break;
            case '\'':
                if (escaped)
                    { value += byte; escaped = false; }
                else
                { 
                    setValue(result, record, key, value);
                    value.clear();
                    state = State::PostValue; 
                }
                break;
            case '\\':
                if (escaped)
                    { value += byte; escaped = false; }
                else 
                    { escaped = true; }
                break;
            case 'n':
                if (escaped)
                    { value += '\n'; escaped = false; }
                else
                    { value += byte; }
                break;
            case 'r':
                if (escaped)
                    { value += '\r'; escaped = false; }
                else
                    { value += byte; }
                break;
            case 't':
                if (escaped)
                    { value += '\t'; escaped = false; }
                else
                    { value += byte; }
                break;
            default:
                if (escaped)
                    { escaped = false; }
                value += byte;
                break;
            }
            break;

        case State::Comment:
        case State::Error:
            switch (byte)
            {
            case '\n':
                state = State::EOL;
                break;
            }
            break;
        }
        pos++;
    }
    buffer.get(pos);
    if (error != Error::Null)
    {
        cpp::String msg = cpp::String::format("BitDecode : error='%' pos='%'", toString(error), pos);
        throw cpp::DecodeException{ msg };
    }
    return result;
}





//  key=(10x)'xxxxxxxxxx'
//  key='xxx'
//  key=null
cpp::String BitData::encodeKeyValue( cpp::Memory dataKey, cpp::Memory dataValue, bool scanValue )
{
    cpp::String encodedValue;
    cpp::Memory value = dataValue;

    if ( value.isNull() )
    {
        return cpp::String::format( "%=null", dataKey );
    }

    bool encodeFlag = false;
    bool bytesFlag = value.length() > 24;

    size_t bytes = value.length( );

    if ( scanValue )
    {
        if ( size_t pos = value.find_first_of( "'\n" ); pos != cpp::String::npos )
        {
            if ( value[pos] == '\n' )
                { encodeFlag = true; }
            else
                { bytesFlag = true; }

            if ( !encodeFlag )
            {
                if ( pos = value.find( '\n', pos + 1 ); pos != cpp::String::npos )
                    { encodeFlag = true; }
            }

            if ( encodeFlag )
            {
                encodedValue = cpp::toBase64( value );
                value = encodedValue;
            }
        }
    }
    bytes = value.length( );

    if ( bytesFlag || encodeFlag )
    {
        return cpp::String::format( "%(%%)='%'",
            dataKey,
            bytes,
            encodeFlag ? "e" : "",
            value );
    }
    else
    {
        return cpp::String::format( "%='%'",
            dataKey,
            value );
    }

    return "";
}

//  key(10x)='xxxxxxxxxx'
//  key='xxx'
//  key=null
BitData::DecodedKeyValue BitData::decodeKeyValue( cpp::ReadBuffer & buffer )
{
    DecodedKeyValue result;

    cpp::Memory encodedDatum = buffer.getable( );

    auto match = encodedDatum.searchOne( R"(\s*(\w+)(?:\((\d+)(e?)\))?=)" );
    cpp::check<cpp::DecodeException>( match.hasMatch(), cpp::String::format( "Unable to decode BitData::Pair : '%'", encodedDatum ) );

    bool isKnownLength = !match[2].isEmpty( );
    size_t dataBytes = isKnownLength
        ? cpp::Integer::parse( match[2] )
        : 0;
    bool encodedFlag = isKnownLength
        ? match[3] == "e"
        : false;

    size_t valueLen = 0;
    cpp::Memory value = encodedDatum.substr( match[0].length( ) );

    if ( isKnownLength )
    {
        valueLen = dataBytes + 2;
        bool isLengthValid = ( value.length( ) >= valueLen );
        bool isDelimited = value[0] == '\'' && value[valueLen - 1] == '\'';
        cpp::check<cpp::DecodeException>( isLengthValid && isDelimited, cpp::String::format( "Unable to decode BitData::Pair : '%'", encodedDatum ) );
        value = value.substr( 1, valueLen - 2 );
    }
    else if ( value.substr( 0, 4 ) == "null" )
    {
        valueLen = 4;
        value = cpp::Memory::Null;
    }
    else
    {
        cpp::check<cpp::DecodeException>( value[0] == '\'', cpp::String::format( "Unable to decode BitData::Pair : '%'", encodedDatum ) );
        auto pos = value.find( '\'', 1 );
        cpp::check<cpp::DecodeException>( pos != cpp::Memory::npos, cpp::String::format( "Unable to decode BitData::Pair : '%'", encodedDatum ) );

        valueLen = pos + 1;
        value = value.substr( 1, valueLen - 2 );
    }

    result.key = match[1];
    result.value.value = value;
    if ( encodedFlag )
    {
        result.value.buffer = cpp::fromBase64( result.value.value );
        result.value.value = result.value.buffer;
    }

    buffer.get( match[0].length( ) + valueLen );

    return result;
}

cpp::String BitData::encode( cpp::Memory id, const cpp::DataMap & data, bool scanValue )
{
    cpp::String record = cpp::String::format( "% :", id );
    if ( data.isEmpty( ) )
    {
        record += " null";
    }
    else
    {
        for ( auto & datum : data )
        {
            record += " " + encodeKeyValue( datum.first, datum.second, scanValue );
        }
    }
    record += '\n';

    return record;
}

cpp::String BitData::encode( const BitData::Record & record, bool scanValue )
{
    return encode( record.key, record.data, scanValue );
}

cpp::String BitData::encodeData( const cpp::DataMap & data, bool scanValue )
{
    cpp::String record;
    for ( auto & datum : data )
    {
        if ( record.empty() == false )
            { record += " "; }
        record += encodeKeyValue( datum.first, datum.second, scanValue );
    }
    return record;
}

BitData::DecodedRecord BitData::decode( cpp::Memory encodedRecord )
{
    DecodedRecord result;

    cpp::ReadBuffer buffer{ encodedRecord };
    auto match = encodedRecord.searchOne( R"(([^:=\s]*)\s*:\s*)" );
    cpp::check<cpp::DecodeException>( match.hasMatch(), cpp::String::format( "Unable to decode BitData::Record : '%'", encodedRecord ) );

    result.key = match[1];

    size_t pos = match[0].length( );
    buffer.get( pos );

    if ( buffer.getable( ).match( R"(null\s*)" ) )
    {
        buffer.get( 4 );
    }
    else
    {
        while ( buffer.getable( ).length( ) > 1 )
        {
            auto kv = decodeKeyValue( buffer );
            result.data[kv.key] = std::move(kv.value);
        }
    }
    if ( buffer.getable( ).get( 0 ) == '\n' )
        { buffer.get( 1 ); }

    return result;
}

cpp::String BitData::encodeInt( int64_t value )
{
    return cpp::Integer::toDecimal( value );
}

cpp::String BitData::encodeHex( uint64_t value )
{
    return cpp::Integer::toHex( value, 0, 0, false, true, false );
}

cpp::String BitData::encodeUint( uint64_t value )
{
    return cpp::Integer::toDecimal( value );
}

cpp::String BitData::encodeF64( f64_t value )
{
    return cpp::String::printf( "%llf", value );
}

cpp::String BitData::encodeBool( bool value )
{
    return value ? "true" : "false";
}

cpp::String BitData::encodeTime( cpp::DateTime time )
{
    time = time.toUTC( );

    auto millis = time.sinceEpoch( ).millis( ) % 1000;
    return cpp::String::format( "%.%", cpp::Date::fromTime( time ).toString( ), cpp::Integer::toDecimal( millis, 3, 0, true ) );
}

int64_t BitData::decodeInt( const cpp::String & text )
{
    return cpp::Integer::parse( text );
}

uint64_t BitData::decodeHex( const cpp::String & text )
{
    return cpp::Integer::parseUnsigned( text, 16 );
}

uint64_t BitData::decodeUint( const cpp::String & text )
{
    return cpp::Integer::parseUnsigned( text );
}

f64_t BitData::decodeF64( const cpp::String & text )
{
    char * p;
    f64_t value = strtod( text.c_str(), &p );
    cpp::check<cpp::DecodeException>( errno != ERANGE, "BitData::decodeF64() : range error while parsing float" );
    return value;
}

bool BitData::decodeBool( const cpp::String & text )
{
    return text == "true";
}

cpp::DateTime BitData::decodeTime( const cpp::String & text )
{
    auto match = text.match( R"((\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2}).(\d{3}))" );
    cpp::check<cpp::DecodeException>( match.groups().size( ) == 8, cpp::String::format( "AccountMap::decodeTime() could not parse text: %", text ) );

    auto year = cpp::Integer::parseUnsigned( match[1] );
    auto month = cpp::Integer::parseUnsigned( match[2] );
    auto day = cpp::Integer::parseUnsigned( match[3] );
    auto hour = cpp::Integer::parseUnsigned( match[4] );
    auto min = cpp::Integer::parseUnsigned( match[5] );
    auto sec = cpp::Integer::parseUnsigned( match[6] );
    auto milli = cpp::Integer::parseUnsigned( match[7] );

    cpp::Date date = cpp::Date::create( (int)year, (int)month, (int)day, (int)hour, (int)min, (int)sec );

    return date.toTime( ).fromUTC( ) + cpp::Duration::ofMillis( milli );
}

#else

#include <cpp/meta/Unittest.h>

#include "BitData.h"

SUITE( BitData )
{
    TEST( test1 )
    {
        cpp::Memory value[5] = 
        { 
            "value", 
            "A long value which should be length encoded.", 
            "A value with ' and a \n and anouther '", 
            "A value with \n", 
            "A value with a ' character" 
        };
        cpp::Memory expected[5] =
        { 
            "item[0] : attr='value'\n", 
            "item[1] : attr(44)='A long value which should be length encoded.'\n", 
            "item[2] : attr(50e)='QSB2YWx1ZSB3aXRoICcgYW5kIGEgCiBhbmQgYW5vdXRoZXIgJw'\n", 
            "item[3] : attr(19e)='QSB2YWx1ZSB3aXRoIAo'\n", 
            "item[4] : attr(26)='A value with a ' character'\n" 
        };

        for ( int i = 0; i < 5; i++ )
        {
            auto encoded = BitData::encode( cpp::String::format( "item[%]", i ), { { "attr", value[i] } } );
            CHECK( encoded == expected[i] );

            auto record = BitData::decode( encoded );
            CHECK( record.data["attr"].value == value[i] );
        }
    }
}

#endif
