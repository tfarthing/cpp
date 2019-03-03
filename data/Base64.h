#pragma once

#include "../../cpp/data/Memory.h"
#include "../../cpp/process/Exception.h"

namespace cpp
{
	struct Base64
	{
		static std::string encode( cpp::Memory data, bool usePadding = false );
		static std::string decode( cpp::Memory base64 );

		static int decodeValue( char c, bool allowPadding = true );
	};

	inline std::string Base64::encode( cpp::Memory data, bool usePadding )
	{
		static const char base64digit[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		std::string result;
		result.reserve( data.length( ) * 4 / 3 );

		const char* ptr = data.begin( );
		while ( ptr < data.end( ) )
		{
			size_t blockLen = std::min<size_t>( data.end( ) - ptr, 3 );

			uint8_t p0 = *( ptr );
			uint8_t p1 = ( blockLen > 1 ) ? *( ptr + 1 ) : 0;
			uint8_t p2 = ( blockLen > 2 ) ? *( ptr + 2 ) : 0;

			result += base64digit[p0 >> 2];
			result += base64digit[( ( p0 & 0x03 ) << 4 ) | ( p1 >> 4 )];
			if ( blockLen > 1 || usePadding )
			{
				result += ( blockLen > 1 ? base64digit[( ( p1 & 0x0f ) << 2 ) | ( p2 >> 6 )] : '=' );
			}
			if ( blockLen > 2 || usePadding )
			{
				result += ( blockLen > 2 ? base64digit[p2 & 0x3f] : '=' );
			}

			ptr += blockLen;
		}

		return result;

	}

	inline std::string Base64::decode( cpp::Memory data )
	{
		std::string result;
		result.reserve( data.length( ) );

		const char* ptr = data.begin( );
		while ( ptr != data.end( ) )
		{
			size_t blockLen = std::min<size_t>( data.end( ) - ptr, 4 );

			cpp::check<DecodeException>( blockLen > 1, "cpp::Base64::decode() : invalid input" );

			int c1 = decodeValue( *( ptr + 0 ), false );
			int c2 = decodeValue( *( ptr + 1 ), false );
			cpp::check<DecodeException>( c1 != -1 && c2 != -1, "cpp::Base64::decode() : invalid input" );
			result += (unsigned char)( c1 << 2 | c2 >> 4 );

			if ( blockLen < 3 )
				{ break; }

			int c3 = decodeValue( *( ptr + 2 ) );
			if ( c3 != -1 )
				{ result += (unsigned char)( c2 << 4 | c3 >> 2 ); }

			if ( blockLen < 4 )
				{ break; }

			int c4 = decodeValue( *( ptr + 3 ) );
			if ( c4 != -1 )
				{ result += (unsigned char)( ( ( c3 << 6 ) & 0xc0 ) | c4 ); }

			ptr += blockLen;
		}

		return result;
	}

	inline int Base64::decodeValue( char c, bool allowPadding )
	{
		if ( c >= 'A' && c <= 'Z' )
			{ return c - 'A'; }
		else if ( c >= 'a' && c <= 'z' )
			{ return c - 'a' + 26; }
		else if ( c >= '0' && c <= '9' )
			{ return c - '0' + 26 + 26; }
		else if ( c == '+' )
			{ return 10 + 26 + 26; }
		else if ( c == '/' )
			{ return 10 + 26 + 26 + 1; }
		else if ( allowPadding && c == '=' )
			{ return -1; }

		throw DecodeException( "cpp::Base64::decodeValue() : invalid input" );
	}

}