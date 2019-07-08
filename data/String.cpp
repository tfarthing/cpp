#ifndef TEST

#include <cstdarg>
#include <algorithm>

#include "String.h"



namespace cpp
{

    String & String::replaceAll( const Memory & pattern, const Memory & dst, size_t pos )
    {
        while ( pos < length( ) )
        {
            size_t offset = data.find( pattern.data(), pos, pattern.length() );
            if ( offset == npos )
				{ break; }
			data.replace( offset, pattern.length( ), dst.data(), dst.length() );
            pos = offset + dst.length( );
        }
        return *this;
    }


    String & String::replaceFirst( const Memory & pattern, const Memory & dst, size_t pos )
    {
        size_t offset = data.find( pattern.data( ), pos, pattern.length( ) );
        if ( offset != npos )
        {
			data.replace( offset, pattern.length( ), dst.data( ), dst.length( ) );
        }
        return *this;
    }


    String & String::replaceLast( const Memory & pattern, const Memory & dst, size_t pos )
    {
        size_t offset = data.rfind( pattern.data( ), pos, pattern.length( ) );
        if ( offset != npos )
        {
			data.replace( offset, pattern.length( ), dst.data( ), dst.length( ) );
        }
        return *this;
    }


    String & String::toUpper( )
    {
        for ( size_t i = 0; i<length( ); i++ )
        {
            char & ch = data.at( i ); ch = toupper( ch );
        }
        return *this;
    }


    String & String::toLower( )
    {
        for ( size_t i = 0; i<length( ); i++ )
        {
            char & ch = data.at( i ); ch = tolower( ch );
        }
        return *this;
    }


	String String::printf( const char * fmt, ...)
	{
		String result(22, '\0');

		va_list args;
		va_start( args, fmt );

		int len = 0;
		while ( ( len = vsnprintf( (char *)result.begin( ), result.length( ), fmt, args ) ) >= result.length( ) )
		    { result.resize( len + 1 ); }
		if ( len < 0 )
			{ throw std::exception("Encoding error occured in String::format."); }
		result.resize( len );

		va_end(args);

		return result;
	}

}

#else

#include <cpp/data/String.h>
#include <cpp/text/Utf8.h>
#include <cpp/text/Utf16.h>
#include <cpp/meta/Test.h>

TEST_CASE( "String" )
{
    using namespace cpp;

    SECTION( "trim" )
    {
        REQUIRE( String{ " hello " }.trim( ) == "hello" );
    }

    SECTION( "split" )
    {
        StringArray lhs = String{ "one, two, three" }.split( "," );
        StringArray rhs = StringArray{ "one", "two", "three" };
        REQUIRE( lhs == rhs );
    }

    SECTION( "utf16" )
    {
        String string = u8"ありがとう";
        
        std::wstring utf16 = toUtf16( string );
        String utf8 = toUtf8( utf16 );

        REQUIRE( utf8 == string );
    }

}

#endif