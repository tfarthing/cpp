#pragma once

#include "../../cpp/data/Memory.h"

namespace cpp
{
	struct Hex
	{
		static std::string encode( cpp::Memory data, bool caseUpper = false, bool reverse = false );
		static std::string decode( cpp::Memory base64, bool reverse = false );
	};

	inline std::string Hex::encode( cpp::Memory src, bool caseUpper, bool reverse )
	{
		static char hexUpper[17] = "0123456789ABCDEF";
		static char hexLower[17] = "0123456789abcdef";

		std::string result;
		result.reserve( src.length( ) * 2 );

		for ( size_t i = 0; i < src.length( ); i++ )
		{
			uint8_t c = reverse ? src[src.length( ) - ( i + 1 )] : src[i];
			result += ( caseUpper ? hexUpper[( c & 0xf0 ) >> 4] : hexLower[( c & 0xf0 ) >> 4] );
			result += ( caseUpper ? hexUpper[c & 0x0f] : hexLower[c & 0x0f] );
		}
		return result;
	}

	inline std::string Hex::decode(cpp::Memory src, bool reverse )
	{
		std::string result;
		result.reserve( src.length( ) / 2 );

		for ( size_t i = 0; i < src.length( ); i += 2 )
		{
			uint8_t c;
			char hex1 = reverse ? src[src.length( ) - ( i + 2 )] : src[i];
			char hex2 = reverse ? src[src.length( ) - ( i + 1 )] : src[i + 1];
			
			if ( hex1 >= '0' && hex1 <= '9' )
				{ c = hex1 - '0'; }
			else if ( hex1 >= 'a' && hex1 <= 'f' )
				{ c = hex1 - 'a' + 10; } 
			else if ( hex1 >= 'A' && hex1 <= 'F' )
				{ c = hex1 - 'A' + 10; }
			else
				{ break; }
			
			c <<= 4;
			
			if ( hex2 >= '0' && hex2 <= '9' )
				{ c |= hex2 - '0'; }
			else if ( hex2 >= 'a' && hex2 <= 'f' )
				{ c |= hex2 - 'a' + 10; }
			else if ( hex2 >= 'A' && hex2 <= 'F' )
				{ c |= hex2 - 'A' + 10; }
			else
				{ break; }
			
			result += c;
		}
		return result;
	}

}
