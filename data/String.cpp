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
            size_t offset = find( pattern.data(), pos, pattern.length() );
            if ( offset == npos )
				{ break; }
            replace( offset, pattern.length( ), dst.data(), dst.length() );
            pos = offset + dst.length( );
        }
        return *this;
    }

    String & String::replaceFirst( const Memory & pattern, const Memory & dst, size_t pos )
    {
        size_t offset = find( pattern.data( ), pos, pattern.length( ) );
        if ( offset != npos )
        {
            replace( offset, pattern.length( ), dst.data( ), dst.length( ) );
        }
        return *this;
    }

    String & String::replaceLast( const Memory & pattern, const Memory & dst, size_t pos )
    {
        size_t offset = rfind( pattern.data( ), pos, pattern.length( ) );
        if ( offset != npos )
        {
            replace( offset, pattern.length( ), dst.data( ), dst.length( ) );
        }
        return *this;
    }

    String & String::toUpper( )
    {
        for ( size_t i = 0; i<length( ); i++ )
        {
            char & ch = at( i ); ch = toupper( ch );
        }
        return *this;
    }

    String & String::toLower( )
    {
        for ( size_t i = 0; i<length( ); i++ )
        {
            char & ch = at( i ); ch = tolower( ch );
        }
        return *this;
    }

	String String::printf(const char * fmt, ...)
	{
		String result(22, '\0');

		va_list args;
		va_start(args, fmt);

		int len = 0;
		while ( ( len = vsnprintf( (char *)result.c_str(), result.length(), fmt, args ) ) >= result.length() )
		    { result.resize( len + 1, '\0' ); }
		if (len < 0)
			{ throw std::exception("Encoding error occured in String::format."); }
		result.resize(len);

		va_end(args);

		return result;
	}

}

#else

#include <cpp/String.h>
#include <cpp/util/Utf8.h>
#include <cpp/util/Utf16.h>
#include <cpp/meta/Unittest.h>

SUITE( String )
{
    using namespace cpp;

    TEST( trim )
    {
        CHECK( String{ " hello " }.trim( ) == "hello" );
    }

    TEST( split )
    {
        auto lhs = String{ "one, two, three" }.split( "," );
        auto rhs = String::Array{ "one", "two", "three" };
        CHECK( lhs == rhs );
    }

    TEST( utf16 )
    {
        String string = u8"ありがとう";
        
        std::wstring utf16 = toUtf16( string );
        String utf8 = toUtf8( utf16 );

        CHECK( utf8 == string );
    }

}

#endif