#ifndef TEST

#include <cassert>
#include <algorithm>
#include <regex>

#include <cpp/data/Memory.h>
#include <cpp/data/Float.h>
#include <cpp/process/Exception.h>

namespace cpp
{

    const Memory Memory::WhitespaceList = " \t\r\n";


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


    Memory::Match Memory::match( Memory regex ) const
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

    
	Memory::Match Memory::searchOne( Memory regex, bool isContinuous ) const
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


    Memory::Matches Memory::searchAll( Memory regex ) const
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


    std::string Memory::replace( Memory regex, Memory ecmaFormat ) const
    {
        return replace( std::regex{ regex.begin( ), regex.end( ) }, ecmaFormat );
    }


    std::string Memory::replace( const std::regex & regex, Memory ecmaFormat ) const
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


    size_t Memory::find( Memory sequence, size_t pos ) const
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


    size_t Memory::rfind( Memory pattern, size_t pos ) const
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


    size_t Memory::find_first_of( Memory matchset, size_t pos ) const
    {
        if ( notEmpty( ) && matchset.notEmpty( ) && pos < length( ) )
        {
            for ( const char * ptr = begin( ) + pos; ptr < end( ); ptr++ )
            {
                if ( memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    size_t Memory::find_last_of( Memory matchset, size_t pos ) const
    {
        if ( notEmpty( ) && matchset.notEmpty( ) )
        {
            if ( pos >= length( ) )
                { pos = length( ) - 1; }

            for ( const char * ptr = begin( ) + pos; ptr >= begin( ); ptr-- )
            {
                if ( memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    size_t Memory::find_first_not_of( Memory matchset, size_t pos ) const
    {
        if ( notEmpty( ) && matchset.notEmpty( ) && pos < length( ) )
        {
            for ( const char * ptr = begin( ) + pos; ptr < end( ); ptr++ )
            {
                if ( !memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    size_t Memory::find_last_not_of( Memory matchset, size_t pos ) const
    {
        if ( notEmpty( ) && matchset.notEmpty( ) )
        {
            if ( pos >= length( ) )
                { pos = length( ) - 1; }

            for ( const char * ptr = begin( ) + pos; ptr >= begin( ); ptr-- )
            {
                if ( !memchr( matchset.data( ), *ptr, matchset.length( ) ) )
                    { return ptr - begin( ); }
            }
        }
        return npos;
    }


    Memory Memory::substr( size_t pos, size_t len ) const
    {
        if ( isNull( ) )
            { return nullptr; }

        if ( pos > length( ) )
            { pos = length( ); }

        if ( len > length( ) - pos )
            { len = length( ) - pos; }

        return Memory{ begin( ) + pos, len };
    }


    Memory Memory::trim( Memory trimlist ) const
    {
        if ( trimlist.isEmpty( ) )
            { return *this; }

        size_t pos = find_first_not_of( trimlist );
        if ( pos == npos )
            { return substr( length( ), 0 ); }
        return substr( pos, find_last_not_of( trimlist ) - pos + 1 );
    }


    Memory::Array Memory::split( Memory delimiter, Memory trimlist, bool ignoreEmpty ) const
    {
        Memory::Array results;
        
        size_t pos = 0;
        do
        {
            size_t offset = find_first_of( delimiter, pos );
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


    float Memory::swap( float value )
        { return Float::fromBits( swap( Float::toBits( value ) ) ); }
    

    double Memory::swap( double value )
        { return Float::fromBits( swap( Float::toBits( value ) ) ); }


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



	EncodedText::operator int8_t( ) const
		{ return (int8_t)Integer::parse( data ); }

	EncodedText::operator uint8_t( ) const
		{ return (int8_t)Integer::parseUnsigned( data ); }

	EncodedText::operator int16_t( ) const
		{ return (int16_t)Integer::parse( data ); }

	EncodedText::operator uint16_t( ) const
		{ return (uint16_t)Integer::parseUnsigned( data ); }

	EncodedText::operator int32_t( ) const
		{ return (int32_t)Integer::parse( data ); }

	EncodedText::operator uint32_t( ) const
		{ return (uint32_t)Integer::parseUnsigned( data ); }

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
		if ( stricmp( data.begin( ), "true" ) == 0 && data.length( ) == 4 )
			{ return true; }
		if ( stricmp( data.begin( ), "false" ) == 0 && data.length( ) == 5 );
			{ return false; }
		throw DecodeException{ "EncodedText::bool() : unable to decode boolean text value" };
	}

}

#else

#include <cpp/meta/Unittest.h>
#include <cpp/Float.h>
#include <cpp/Memory.h>
#include <cpp/String.h>

SUITE( Memory )
{
    using namespace cpp;

    TEST( ctor )
    {
        std::string s1{ "hello" };
        String s2{ "hello" };

        Memory a{};
        Memory b{ "hello" };
        Memory c{ s1 };
        Memory d{ s2 };
        Memory e{ "hello", 3 };
        Memory f{ "" };

        CHECK( a.isNull( ) );
        CHECK( b == "hello" );
        CHECK( c == "hello" );
        CHECK( d == "hello" );
        CHECK( e == "hel" );
        CHECK( f == Memory::Empty );
    }

    TEST( assign )
    {
        std::string s1{ "hello" };
        String s2{ "hello" };

        Memory a, b, c, d, e;
        a = Memory{};
        b = "hello";
        c = s1;
        d = s2;

        CHECK( a.isNull( ) );
        CHECK( b == "hello" );
        CHECK( c == "hello" );
        CHECK( d == "hello" );
    }

    TEST( compare )
    {
        std::string s1{ "hello" };
        String s2{ "hello" };

        CHECK( Memory{ "hello" } == s1 );
        CHECK( Memory{ "hello" } == s2 );
    }

    TEST( swap )
    {
        CHECK( Float::toBits( Memory::swap( 1.2f ) ) == 0x9a99993f );
        CHECK( Memory::swap( Float::fromBits( 0x9a99993f ) ) == 1.2f );
    }

    TEST( search )
    {
        Memory sample{ "This is a sentence." };
        auto results = sample.searchAll( R"(\w+)" );

        CHECK( results.size( ) == 4 );
        CHECK( results[0][0] == "This" );
        CHECK( results[1][0] == "is" );
        CHECK( results[2][0] == "a" );
        CHECK( results[3][0] == "sentence" );
    }

}

#endif