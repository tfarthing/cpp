#ifndef TEST

#include <cassert>
#include <cstdarg>
#include <algorithm>
#include <regex>

#include <cpp/data/Memory.h>
#include <cpp/data/Integer.h>
#include <cpp/data/Float.h>
#include <cpp/data/Hex.h>
#include <cpp/data/Base64.h>
#include <cpp/data/DataBuffer.h>
#include <cpp/process/Exception.h>

namespace cpp
{

	const Memory Memory::Empty					= "";
	const Memory Memory::WhitespaceList			= " \t\r\n";



	Memory::Memory( const String & str )
		: m_begin( str.begin( ) ), m_end( str.end( ) ) { }


	Memory & Memory::operator=( const String & str )
		{ m_begin = str.begin( ); m_end = str.end( ); return *this; }


	int	Memory::compare( const Memory & lhs, const Memory & rhs )
	{
		// treat null like empty string
		if ( lhs.isNull( ) )
			{ return rhs.isNull( ) ? 0 : -1; }
		if ( rhs.isNull( ) )
			{ return 1; }

		// use strcmp if either side is null terminated
		if ( !rhs.end( ) && !lhs.end( ) )
			{ return strcmp( lhs.data( ), rhs.data( ) ); }

		// otherwise use memcmp
		size_t len1 = lhs.length( );
		size_t len2 = rhs.length( );
		int result = memcmp( lhs.data( ), rhs.data( ), std::min( len1, len2 ) );
		if ( result == 0 && len1 != len2 )
			{ return ( len1 < len2 ) ? -1 : 1; }
		return result;
	}


    Memory::Match Memory::match( const Memory & regex ) const
    {
        return match( std::regex{ regex.begin( ), regex.end( ) } );
    }


    Memory::Match Memory::match( const std::regex & regex ) const
    {
        Memory::Match result;

        std::cmatch matchResults;
        if ( std::regex_match( begin( ), end( ), matchResults, regex ) )
            { result = matchResults; }

        return result;
    }

    
	Memory::Match Memory::searchOne( const Memory & regex, bool isContinuous ) const
    {
        return searchOne( std::regex{ regex.begin( ), regex.end( ) }, isContinuous );
    }


    Memory::Match Memory::searchOne( const std::regex & regex, bool isContinuous ) const
    {
        Memory::Match result;

        std::cmatch matchResults;
        std::regex_constants::match_flag_type matchFlag = isContinuous
            ? std::regex_constants::match_continuous
            : std::regex_constants::match_default;
        if ( std::regex_search( begin( ), end( ), matchResults, regex, matchFlag ) )
            { result = matchResults; }

        return result;
    }


    Memory::Matches Memory::searchAll( const Memory & regex ) const
    {
        return searchAll( std::regex{ regex.begin( ), regex.end( ) } );
    }


    Memory::Matches Memory::searchAll( const std::regex & regex ) const
    {
        Memory::Matches results;

        const char * pos = begin( );
        while ( pos < end( ) )
        {
            Memory::Match result;
            std::cmatch matchResults;
            if ( !std::regex_search( pos, end( ), matchResults, regex ) )
                { break; }

            pos = matchResults.suffix( ).first;
            results.emplace_back( matchResults );
        }

        return results;
    }


    std::string Memory::replace( const Memory & regex, const Memory & ecmaFormat ) const
    {
        return replace( std::regex{ regex.begin( ), regex.end( ) }, ecmaFormat );
    }


    std::string Memory::replace( const std::regex & regex, const Memory & ecmaFormat ) const
    {
        std::string result;
        std::regex_replace( std::back_inserter( result ), begin( ), end( ), regex, ecmaFormat.begin( ), std::regex_constants::match_default );
        return result;
    }


    size_t Memory::find( char ch, size_t pos ) const
    {
        if ( notEmpty( ) )
        {
            for ( const char * ptr = begin( ) + pos; ptr < end( ); ptr++ )
            {
                if ( *ptr == ch )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    size_t Memory::find( const Memory & sequence, size_t pos ) const
    {
        if ( length( ) >= sequence.length( ) && sequence.notEmpty( ) )
        {
            const char * last = end( ) - sequence.length( );
            for ( const char * ptr = begin( ) + pos; ptr <= last; ptr++ )
            {
                if ( sequence == Memory{ ptr, sequence.length( ) } )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    size_t Memory::rfind( char ch, size_t pos ) const
    {
        if ( notEmpty( ) )
        {
			if ( pos > length( ) - 1 )
				{ pos = length( ) - 1; }

			for ( const char * ptr = begin( ) + pos; ptr >= begin( ); ptr-- )
            {
                if ( *ptr == ch )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    size_t Memory::rfind( const Memory & pattern, size_t pos ) const
    {
        if ( length( ) >= pattern.length( ) && !pattern.isEmpty( ) )
        {
			if ( pos > length( ) - pattern.length( ) )
				{ pos = length( ) - pattern.length( ); }

            for ( const char * ptr = begin( ) + pos; ptr >= begin( ); ptr-- )
            {
                if ( pattern == Memory{ ptr, pattern.length( ) } )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    size_t Memory::findFirstOf( const Memory & matchset, size_t pos ) const
    {
        if ( pos > length( ) )
            { pos = length( ); }

        for ( const char * ptr = begin( ) + pos; ptr < end( ); ptr++ )
        {
            if ( memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                { return ptr - begin( ); }
        }

        return npos;
    }


    size_t Memory::findLastOf( const Memory & matchset, size_t pos ) const
    {
        if ( pos >= length( ) )
            { pos = length( ) - 1; }

        for ( const char * ptr = begin( ) + pos; ptr >= begin( ); ptr-- )
        {
            if ( memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                { return ptr - begin( ); }
        }

        return npos;
    }


    size_t Memory::findFirstNotOf( const Memory & matchset, size_t pos ) const
    {
        if ( pos > length( ) )
            { pos = length( ); }

        for ( const char * ptr = begin( ) + pos; ptr < end( ); ptr++ )
        {
            if ( !memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                { return ptr - begin( ); }
        }

        return npos;
    }


    size_t Memory::findLastNotOf( const Memory & matchset, size_t pos ) const
    {
        if ( pos >= length( ) )
            { pos = length( ) - 1; }

        for ( const char * ptr = begin( ) + pos; ptr >= begin( ); ptr-- )
        {
            if ( !memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                { return ptr - begin( ); }
        }

        return npos;
    }


	std::string	Memory::replaceFirst( const Memory & sequence, const Memory & dst, size_t pos ) const
	{
		std::string result = *this;
		size_t offset = result.find( sequence.data( ), pos, sequence.length( ) );
		if ( offset != npos )
		{
			result.replace( offset, sequence.length( ), dst.data( ), dst.length( ) );
		}
		return result;
	}


	std::string	Memory::replaceLast( const Memory & sequence, const Memory & dst, size_t pos ) const
	{
		std::string result = *this;
		size_t offset = result.rfind( sequence.data( ), pos, sequence.length( ) );
		if ( offset != npos )
		{
			result.replace( offset, sequence.length( ), dst.data( ), dst.length( ) );
		}
		return result;
	}


	std::string	Memory::replaceAll( const Memory & sequence, const Memory & dst, size_t pos ) const
	{
		std::string result = *this;
		while ( pos < length( ) )
		{
			size_t offset = result.find( sequence.data( ), pos, sequence.length( ) );
			if ( offset == npos )
			{
				break;
			}
			result.replace( offset, sequence.length( ), dst.data( ), dst.length( ) );
			pos = offset + dst.length( );
		}
		return result;
	}


    Memory Memory::substr( size_t pos, size_t len ) const
    {
        if ( isNull( ) )
            { return nullptr; }

        auto maxlen = length( );
        if ( pos > maxlen )
            { pos = maxlen; }

        if ( len > maxlen - pos )
            { len = maxlen - pos; }

        return Memory{ begin( ) + pos, len };
    }


    Memory Memory::trim( const Memory & trimlist ) const
    {
        return trimFront( trimlist ).trimBack( trimlist );
    }


    Memory Memory::trimFront( const Memory & trimlist ) const
    {
        size_t pos = findFirstNotOf( trimlist );
        if ( pos == npos )
            { return substr( length( ), 0 ); }
        return substr( pos );
    }


    Memory Memory::trimBack( const Memory & trimlist ) const
    {
        size_t pos = findLastNotOf( trimlist );
        if ( pos == npos )
            { return substr( 0, 0 ); }
        return substr( 0, pos + 1 );
    }


    Memory::Array Memory::split( const Memory & delimiter, const Memory & trimlist, bool ignoreEmpty ) const
    {
        Memory::Array results;
        
        size_t pos = 0;
        do
        {
            size_t offset = findFirstOf( delimiter, pos );
            if ( offset == npos )
                { offset = length( ); }
            Memory str = substr( pos, offset - pos ).trim( trimlist );
            if ( str.notEmpty( ) || !ignoreEmpty )
                { results.push_back( str ); }
            pos = offset + 1;
        } 
        while ( pos <= length( ) );

        return results;
    }


    float Memory::byteswap( float value )
        { return Float::fromBits( byteswap( Float::toBits( value ) ) ); }
    

    double Memory::byteswap( double value )
        { return Float::fromBits( byteswap( Float::toBits( value ) ) ); }


    bool Memory::isEscaped( size_t pos ) const
    {
        bool escapeFlag = false;
        while ( pos > 0 && at(--pos) == '\\' )
            { escapeFlag = !escapeFlag; }
        return escapeFlag;
    }


    Memory Memory::copy( Memory & dst, const Memory & src )
    {
		assert( dst.length( ) >= src.length( ) );
        memcpy( (char *)dst.begin(), src.begin(), src.length() );
		return Memory{ dst.begin( ), src.length( ) };
    }


	Memory Memory::move( Memory & dst, const Memory & src )
	{
		assert( dst.length( ) >= src.length( ) );
		memmove( (char *)dst.begin( ), src.begin( ), src.length( ) );
		return Memory{ dst.begin( ), src.length( ) };
	}


	Memory Memory::printf( const char * fmt, ... )
	{
		va_list args;
		va_start( args, fmt );

		int len = vsnprintf( data( ), length( ), fmt, args );
		check<OutOfBoundsException>( len >= 0, "Memory::printf( ) : insufficient buffer space" );
		
		va_end( args );

		return substr( 0, len );
	}


	Memory Memory::format( size_t pos, const Memory & fmt )
	{
		auto buffer = DataBuffer::writeTo( substr( pos ) );
		return buffer.put( fmt );
	}



	EncodedText::operator int8_t( ) const
		{ return Integer::to<int8_t>( Integer::parse( data ) ); }


	EncodedText::operator uint8_t( ) const
		{ return Integer::to<uint8_t>( Integer::parseUnsigned( data ) ); }


	EncodedText::operator int16_t( ) const
		{ return Integer::to<int16_t>( Integer::parse( data ) ); }


	EncodedText::operator uint16_t( ) const
		{ return Integer::to<uint16_t>( Integer::parseUnsigned( data ) ); }


	EncodedText::operator int32_t( ) const
		{ return Integer::to<int32_t>( Integer::parse( data ) ); }


	EncodedText::operator uint32_t( ) const
		{ return Integer::to<uint32_t>( Integer::parseUnsigned( data ) ); }


	EncodedText::operator int64_t( ) const
		{ return Integer::parse( data ); }


	EncodedText::operator uint64_t( ) const
		{ return Integer::parseUnsigned( data ); }


	EncodedText::operator float( ) const
		{ return (float)Float::parse( data ); }


	EncodedText::operator double( ) const
		{ return Float::parse( data ); }


	EncodedText::operator bool( ) const
	{  
		if ( _stricmp( data.begin( ), "true" ) == 0 && data.length( ) == 4 )
			{ return true; }
		if ( _stricmp( data.begin( ), "false" ) == 0 && data.length( ) == 5 )
			{ return false; }
		throw DecodeException{ "EncodedText::bool() : unable to decode boolean text value" };
	}



	EncodedDecimal::operator int8_t( ) const
		{ return Integer::to<int8_t>( Integer::parse( data ) ); }


	EncodedDecimal::operator uint8_t( ) const
		{ return Integer::to<uint8_t>( Integer::parseUnsigned( data ) ); }


	EncodedDecimal::operator int16_t( ) const
		{ return Integer::to<int16_t>( Integer::parse( data ) ); }


	EncodedDecimal::operator uint16_t( ) const
		{ return Integer::to<uint16_t>( Integer::parseUnsigned( data ) ); }


	EncodedDecimal::operator int32_t( ) const
		{ return Integer::to<int32_t>( Integer::parse( data ) ); }


	EncodedDecimal::operator uint32_t( ) const
		{ return Integer::to<uint32_t>( Integer::parseUnsigned( data ) ); }


	EncodedDecimal::operator int64_t( ) const
		{ return Integer::parse( data ); }


	EncodedDecimal::operator uint64_t( ) const
		{ return Integer::parseUnsigned( data ); }


	EncodedDecimal::operator float( ) const
		{ return (float)Float::parse( data ); }


	EncodedDecimal::operator double( ) const
		{ return Float::parse( data ); }



	EncodedHex::operator uint8_t( ) const
		{ return Integer::to<uint8_t>( Integer::parseUnsigned( data, 16 ) ); }


	EncodedHex::operator uint16_t( ) const
		{ return Integer::to<uint16_t>( Integer::parseUnsigned( data, 16 ) ); }


	EncodedHex::operator uint32_t( ) const
		{ return Integer::to<uint32_t>( Integer::parseUnsigned( data, 16 ) ); }


	EncodedHex::operator uint64_t( ) const
		{ return Integer::parseUnsigned( data, 16 ); }


	EncodedHex::operator int8_t( ) const
		{ return Integer::to<uint8_t>( Integer::parse( data, 16 ) ); }


	EncodedHex::operator int16_t( ) const
		{ return Integer::to<uint16_t>( Integer::parse( data, 16 ) ); }


	EncodedHex::operator int32_t( ) const
		{ return Integer::to<uint32_t>( Integer::parse( data, 16 ) ); }


	EncodedHex::operator int64_t( ) const
		{ return Integer::parse( data, 16 ); }


	EncodedHex::operator std::string( ) const
		{ return Hex::decode( data ); }



	EncodedBase64::operator std::string( ) const
		{ return Base64::decode( data ); }



	EncodedBinary::operator int8_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<int8_t>( byteOrder ); }


	EncodedBinary::operator uint8_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<uint8_t>( byteOrder ); }


	EncodedBinary::operator int16_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<int16_t>( byteOrder ); }


	EncodedBinary::operator uint16_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<uint16_t>( byteOrder ); }


	EncodedBinary::operator int32_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<int32_t>( byteOrder ); }


	EncodedBinary::operator uint32_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<uint32_t>( byteOrder ); }


	EncodedBinary::operator int64_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<int64_t>( byteOrder ); }


	EncodedBinary::operator uint64_t( ) const
        { return DataBuffer::readFrom( data ).getBinary<uint64_t>( byteOrder ); }


	EncodedBinary::operator float( ) const
        { return DataBuffer::readFrom( data ).getBinary<float>( byteOrder ); }


	EncodedBinary::operator double( ) const
        { return DataBuffer::readFrom( data ).getBinary<double>( byteOrder ); }


	EncodedBinary::operator bool( ) const
		{ return DataBuffer::readFrom( data ).getBinary<bool>( byteOrder ); }



	std::string toString( const String & str )
	{
		return str;
	}


    std::string toString( const std::vector<String> & array )
    { 
        std::string result; 
        for ( auto & element : array )
        {
            if ( !result.empty() )
                { result += ", "; }
            result += element;
        }
        return result;
    }

}

#else

#include <cpp/meta/Test.h>
#include <cpp/data/Float.h>
#include <cpp/data/Memory.h>
#include <cpp/data/String.h>


TEST_CASE( "Memory" )
{
    using namespace cpp;

    SECTION( "ctor" )
    {
        std::string s1{ "hello" };
        String s2{ "hello" };

        Memory a{};
        Memory aa = nullptr;
        Memory b{ "hello" };
        Memory c{ s1 };
        Memory d{ s2 };
        Memory e{ "hello", 3 };
        Memory f{ "" };
        //Memory g{ std::string{""} };      // bad
        //Memory h{ String{""} };           // bad

        REQUIRE( a.isNull( ) );
        REQUIRE( b == "hello" );
        REQUIRE( c == "hello" );
        REQUIRE( d == "hello" );
        REQUIRE( e == "hel" );
        REQUIRE( f == Memory::Empty );
    }

    SECTION( "assign" )
    {
        std::string s1{ "hello" };
        String s2{ "hello" };

        Memory a, b, c, d, e;
        a = Memory{};
        a = nullptr;
        b = "hello";
        c = s1;
        d = s2;

        REQUIRE( a.isNull( ) );
        REQUIRE( b == "hello" );
        REQUIRE( c == "hello" );
        REQUIRE( d == "hello" );
    }

    SECTION( "compare" )
    {
        Memory s0 = "hello";
        std::string s1 = s0;
        String s2 = s0;

        REQUIRE( s0 == s1 );
        REQUIRE( s1 == s0 );
        REQUIRE( s0 == s2 );
        REQUIRE( s2 == s0 );
        REQUIRE( s0 == "hello" );
        REQUIRE( "hello" == s0 );

        s0 = "goodbye";
        REQUIRE( s0 != s1 );
        REQUIRE( s1 != s0 );
        REQUIRE( s0 != s2 );
        REQUIRE( s2 != s0 );
        REQUIRE( s0 != "hello" );
        REQUIRE( "hello" != s0 );

        s0 = "hell";
        REQUIRE( s0 < s1 );
        REQUIRE( s0 <= s1 );
        REQUIRE( s1 > s0 );
        REQUIRE( s1 >= s0 );
        REQUIRE( s0 < s2 );
        REQUIRE( s0 <= s2 );
        REQUIRE( s2 > s0 );
        REQUIRE( s2 >= s0 );
        REQUIRE( s0 < "hello" );
        REQUIRE( s0 <= "hello" );
        REQUIRE( "hello" > s0 );
        REQUIRE( "hello" >= s0 );

        s0 = "hello!";
        REQUIRE( s0 > s1 );
        REQUIRE( s0 >= s1 );
        REQUIRE( s1 < s0 );
        REQUIRE( s1 <= s0 );
        REQUIRE( s0 > s2 );
        REQUIRE( s0 >= s2 );
        REQUIRE( s2 < s0 );
        REQUIRE( s2 <= s0 );
        REQUIRE( s0 > "hello" );
        REQUIRE( s0 >= "hello" );
        REQUIRE( "hello" < s0 );
        REQUIRE( "hello" <= s0 );

        REQUIRE( s0 != nullptr );
        REQUIRE( nullptr != s0 );

        s0 = nullptr;
        REQUIRE( s0 == nullptr );
        REQUIRE( nullptr == s0 );
        REQUIRE( s0 < "hello" );
        REQUIRE( s0 < s1 );
        REQUIRE( s0 < s2 );
    }

    SECTION( "byteswap" )
    {
        REQUIRE( Float::toBits( Memory::byteswap( 1.2f ) ) == 0x9a99993f );
        REQUIRE( Memory::byteswap( Float::fromBits( 0x9a99993f ) ) == 1.2f );
    }

    SECTION( "search" )
    {
        Memory sample{ "This is a sentence." };
        auto results = sample.searchAll( R"(\w+)" );

        REQUIRE( results.size( ) == 4 );
        REQUIRE( results[0][0] == "This" );
        REQUIRE( results[1][0] == "is" );
        REQUIRE( results[2][0] == "a" );
        REQUIRE( results[3][0] == "sentence" );
    }

    SECTION( "format" )
    {
        char buffer[32];

        Memory str{ buffer, 32 };
        REQUIRE( str.printf( "%d %f", 10, 100.001 ) == "10 100.001000");
        REQUIRE( str.format( "% %", 10, 100.001 ) == "10 100.001");

        REQUIRE( cpp::format( "% %", 10, 100.001 ) == "10 100.001" );
        REQUIRE( cpp::format( "% % %", 10, 100.001, "some other extra long text to force alloc" ) == "10 100.001 some other extra long text to force alloc" );
    }

    SECTION( "encoding" )
    {
        int v1 = Memory{ "3a" }.asHex( );
        REQUIRE( v1 == 0x3a );
        uint8 v2 = Memory{ "3a" }.asHex( );
        REQUIRE( v2 == 0x3a );
        int8 v3 = Memory{ "3a" }.asHex( );
        REQUIRE( v3 == 0x3a );

        v1 = Memory{ "1001001" }.asDecimal( );
        REQUIRE( v1 == 1001001 );
        float64 v4 = Memory{ "1001001.01" }.asDecimal( );
        REQUIRE( v4 == 1001001.01 );
        v4 = Memory{ "1001001" }.asDecimal( );
        REQUIRE( v4 == 1001001 );
    }

}

#endif