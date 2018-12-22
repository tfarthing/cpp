#ifndef TEST

#include <algorithm>

#include <cpp/Exception.h>
#include <cpp/util/encoding.h>

namespace cpp
{

    String toURI( const Memory & src )
    {
        String result;
        const char * ptr = src.c_str( );
        while ( *ptr )
        {
            size_t offset = strcspn( ptr, " <>#%{}|\\^~[]`;/?:@=&$" );
            result += Memory( ptr, ptr + offset );
            ptr += offset;
            if ( *ptr )
            {
                offset = result.length( );
                result.resize( offset + 4 );
                sprintf_s( (char *)result.c_str( ) + offset, 4, "%%%2.2X", (int)*ptr++ );
                result.resize( offset + 3 );
            }
        }
        return result.c_str( );
    }

    String fromURI( const Memory & src )
    {
        String result;
        size_t offset;
        const char * ptr = src.c_str( );
        while ( ptr[offset = strcspn( ptr, "%" )] )
        {
            char hex[3] = { *( ptr + offset + 1 ), *( ptr + offset + 2 ), 0 };
            result += Memory( ptr, ptr + offset );
            result += (char)atox( hex );
            ptr += offset + 3;
        }
        result.append( ptr );
        return result.c_str( );
    }

    String toHex( const Memory & src, bool caseUpper, bool reverse )
    {
        static char hexUpper[17] = "0123456789ABCDEF";
        static char hexLower[17] = "0123456789abcdef";

        String result;
        for ( size_t i = 0; i<src.length( ); i++ )
        {
            uint8_t c = reverse ? src[src.length( ) - ( i + 1 )] : src[i];
            result += ( caseUpper ? hexUpper[( c & 0xf0 ) >> 4] : hexLower[( c & 0xf0 ) >> 4] );
            result += ( caseUpper ? hexUpper[c & 0x0f] : hexLower[c & 0x0f] );
        }
        return result;
    }

    String fromHex( const Memory & src, bool reverse )
    {
        String result;
        for ( size_t i = 0; i<src.length( ); i += 2 )
        {
            uint8_t c;
            char hex1 = reverse ? src[src.length( ) - ( i + 2 )] : src[i];
            char hex2 = reverse ? src[src.length( ) - ( i + 1 )] : src[i + 1];
            if ( hex1 >= '0' && hex1 <= '9' )
                c = hex1 - '0';
            else if ( hex1 >= 'a' && hex1 <= 'f' )
                c = hex1 - 'a' + 10;
            else if ( hex1 >= 'A' && hex1 <= 'F' )
                c = hex1 - 'A' + 10;
            else
                break;
            c <<= 4;
            if ( hex2 >= '0' && hex2 <= '9' )
                c |= hex2 - '0';
            else if ( hex2 >= 'a' && hex2 <= 'f' )
                c |= hex2 - 'a' + 10;
            else if ( hex2 >= 'A' && hex2 <= 'F' )
                c |= hex2 - 'A' + 10;
            else
                break;
            result += c;
        }
        return result;
    }

    const char base64digit[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    String toBase64( Memory data, bool usePadding )
    {
        String result;
        result.reserve( data.length( ) * 4 / 3 );

        const char * ptr = data.begin( );
        while ( ptr < data.end( ) )
        {
            size_t blockLen = std::min<size_t>( data.end( ) - ptr, 3 );

            uint8_t p0 = *(ptr);
            uint8_t p1 = ( blockLen > 1 ) ? *( ptr + 1 ) : 0;
            uint8_t p2 = ( blockLen > 2 ) ? *( ptr + 2 ) : 0;

            result += base64digit[p0 >> 2];
            result += base64digit[( ( p0 & 0x03 ) << 4 ) | ( p1 >> 4 )];
            if ( blockLen > 1 || usePadding )
                { result += ( blockLen > 1 ? base64digit[( ( p1 & 0x0f ) << 2 ) | ( p2 >> 6 )] : '=' ); }
            if ( blockLen > 2 || usePadding )
                { result += ( blockLen > 2 ? base64digit[p2 & 0x3f] : '=' ); }

            ptr += blockLen;
        }

        return result;
    }

    String fromBase64( Memory data )
    {
        String result;
        result.reserve( data.length( ) );

        auto decodeValue = []( char c, bool allowPadding = true ) -> int
        {
            if ( c >= 'A' && c <= 'Z' )
                { return c - 'A'; }
            if ( c >= 'a' && c <= 'z' )
                { return c - 'a' + 26; }
            if ( c >= '0' && c <= '9' )
                { return c - '0' + 26 + 26; }
            if ( c == '+' )
                { return 10 + 26 + 26; }
            if ( c == '/' )
                { return 10 + 26 + 26 + 1; }
            if ( allowPadding && c == '=' )
                { return -1; }

            throw DecodeException( "cpp::fromBase64() : invalid input" );
        };

        const char * ptr = data.begin( );
        while ( ptr != data.end( ) )
        {
            size_t blockLen = std::min<size_t>( data.end( ) - ptr, 4 );

            if ( blockLen < 2 )
                { throw DecodeException( "cpp::fromBase64() : invalid input" ); }

            int c1 = decodeValue( *(ptr + 0), false );
            int c2 = decodeValue( *(ptr + 1), false );
            result += (unsigned char)( c1 << 2 | c2 >> 4 );

            if ( blockLen < 3 )
                { break; }
            
            int c3 = decodeValue( *(ptr + 2) );
            if ( c3 != -1 )
                { result += (unsigned char)( c2 << 4 | c3 >> 2 ); }

            if ( blockLen < 4 )
                { break; }

            int c4 = decodeValue( *(ptr + 3) );
            if ( c4 != -1 )
            { result += (unsigned char)( ( ( c3 << 6 ) & 0xc0 ) | c4 ); }

            ptr += blockLen;
        }

        return result;
    }

}

#else

#include <cpp/meta/Unittest.h>

#include "encoding.h"

SUITE( encoding )
{
    TEST( base64 )
    {
        CHECK( cpp::toBase64( "" ) == "" );
        CHECK( cpp::fromBase64( "" ) == "" );

        CHECK( cpp::toBase64( "A", true ) == "QQ==" );
        CHECK( cpp::fromBase64( "QQ==" ) == "A" );

        CHECK( cpp::toBase64( "A" ) == "QQ" );
        CHECK( cpp::fromBase64( "QQ" ) == "A" );

        CHECK( cpp::toBase64( "AB", true ) == "QUI=" );
        CHECK( cpp::fromBase64( "QUI=" ) == "AB" );

        CHECK( cpp::toBase64( "AB" ) == "QUI" );
        CHECK( cpp::fromBase64( "QUI" ) == "AB" );

        CHECK( cpp::toBase64( "ABC" ) == "QUJD" );
        CHECK( cpp::fromBase64( "QUJD" ) == "ABC" );

        CHECK( cpp::toBase64( "ABCD" ) == "QUJDRA" );
        CHECK( cpp::fromBase64( "QUJDRA" ) == "ABCD" );

        cpp::String allValues;
        for ( int i = 0; i < 256; i++ )
            { allValues += (char)i; }
        auto allEncoded = cpp::toBase64( allValues, true );

        CHECK( allEncoded == 
            "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIj"
            "JCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+P0BBQkNERUZH"
            "SElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWpr"
            "bG1ub3BxcnN0dXZ3eHl6e3x9fn+AgYKDhIWGh4iJiouMjY6P"
            "kJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKz"
            "tLW2t7i5uru8vb6/wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX"
            "2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w==" );

        auto allDecoded = cpp::fromBase64( allEncoded );
        CHECK( allDecoded.length( ) == 256 );
        for ( int i = 0; i < 256; i++ )
        {
            CHECK( allDecoded[i] == (char)i );
        }

    }
}

#endif
