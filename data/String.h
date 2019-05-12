#pragma once

/*

	String is an extension of std::string which adds utility methods commonly associated with strings.

	(1) implements traditional string operations (i.e. substr, split, trim, regex, replace sequence, toUpper, toLower ).
	(2) implements safe string formatting, substitutes '%' with toString(arg)

*/

#include <string>
#include <vector>

#include "Memory.h"



namespace cpp
{

    struct String
	{
											String( );
											String( size_t count, char ch );
											String( const char * cstring );
											String( const char * ptr, size_t count );
											String( const Memory & memory );
											String( const std::string & copy );
											String( const String & copy );
											String( std::string && move ) noexcept;
											String( String && move ) noexcept;

		static const size_t					npos = (size_t)-1;
		typedef std::vector<String>			Array;

		bool								operator<( const String & str ) const;
											operator std::string & ( );
											operator const std::string & ( ) const;

		String &							assign( char ch );
		String &							assign( const Memory & memory );
		String &							assign( const std::string & str );
		String &							assign( const String & str );
		String &							assign( std::string && str ) noexcept;
		String &							assign( String && str ) noexcept;
		String &							assign( const char * cstring );

		String &							operator=( char ch );
		String &							operator=( const Memory & memory );
		String &							operator=( const std::string & str );
		String &							operator=( const String & str );
		String &							operator=( std::string && str ) noexcept;
		String &							operator=( String && str ) noexcept;
		String &							operator=( const char * cstring );

		String &							append( char ch );
		String &							append( size_t count, char ch );
		String &							append( const char * begin );
		String &							append( const char * begin, const char * end );
		String &							append( const Memory & memory );
		String &							append( const std::string & str );
		String &							append( const String & str );

		String &							operator+=( char ch );
		String &							operator+=( const char * begin );
		String &							operator+=( const Memory & memory );
		String &							operator+=( const std::string & str );
		String &							operator+=( const String & str );

		bool								notEmpty( ) const;
		bool								isEmpty( ) const;
		bool								operator!( ) const;				 // isEmpty()
											explicit operator bool( ) const; // notEmpty()

		size_t								length( ) const;
		void								resize( size_t len, char ch = '\0' );
		void								clear( );

		const char *						begin( ) const;
		const char *						end( ) const;

		char								at( size_t pos ) const;
		char								operator[]( size_t pos ) const;

		Memory								substr( size_t pos = 0, size_t len = npos ) const;
		Memory::Array						split( const Memory & delimiter, const Memory & trimlist = Memory::WhitespaceList, bool ignoreEmpty = true ) const;
		Memory								trim( const Memory & trimlist = Memory::WhitespaceList ) const;

		size_t								find( char ch, size_t pos = 0 ) const;
		size_t								find( const Memory & sequence, size_t pos = 0 ) const;
		size_t								rfind( char ch, size_t pos = npos ) const;
		size_t								rfind( const Memory & sequence, size_t pos = npos ) const;
		size_t								findFirstOf( const Memory & matchset, size_t pos = 0 ) const;
		size_t								findLastOf( const Memory & matchset, size_t pos = npos ) const;
		size_t								findFirstNotOf( const Memory & matchset, size_t pos = 0 ) const;
		size_t								findLastNotOf( const Memory & matchset, size_t pos = npos ) const;

		String &							replaceFirst( const Memory & sequence, const Memory & dst, size_t pos = 0 );
		String &							replaceLast( const Memory & sequence, const Memory & dst, size_t pos = npos );
		String &							replaceAll( const Memory & sequence, const Memory & dst, size_t pos = 0 );

		typedef RegexMatch<Memory>			Match;
		typedef std::vector<Match>			Matches;
		Match								match( const Memory & regex ) const;
		Match								match( const std::regex & regex ) const;
		Match								searchOne( const Memory & regex, bool isContinuous = false ) const;
		Match								searchOne( const std::regex & regex, bool isContinuous = false ) const;
		Matches								searchAll( const Memory & regex ) const;
		Matches								searchAll( const std::regex & regex ) const;
		String &							replace( const Memory & regex, const Memory & ecmaFormat );
		String &							replace( const std::regex & regex, const Memory & ecmaFormat );

		String &							toUpper( );
		String &							toLower( );

		bool								isEscaped( size_t pos ) const;

		EncodedText							asText( ) const;
		EncodedDecimal						asDecimal( ) const;
		EncodedHex							asHex( ) const;
		EncodedBase64						asBase64( ) const;
		EncodedBinary						asBinary( ByteOrder byteOrder = ByteOrder::Host ) const;

		static String 						printf( const char * fmt, ... );

		template<typename T, typename... Params>
		static String 						format( const Memory & fmt, const T & param, Params... parameters );
		static String 						format( const Memory & fmt );

		std::string							data;
	};


	inline String::String( )
		: data( ) { }


	inline String::String( size_t count, char ch )
		: data( count, ch ) { }


	inline String::String( const char * s )
		: data( s ) { }


	inline String::String( const char * s, size_t count )
		: data( s, count ) { }


	inline String::String( const Memory & memory )
		: data( memory.begin( ), memory.begin( ) + memory.length( ) ) { }


	inline String::String( const std::string & copy )
		: data( copy ) { }


	inline String::String( const String & copy )
		: data( copy.data ) { }


	inline String::String( std::string && move ) noexcept
		: data( std::move( move ) ) { }


	inline String::String( String && move ) noexcept
		: data( std::move( move.data ) ) { }


	inline bool String::operator<( const String & str ) const
		{ return data < str.data; }


	inline String::operator std::string &( )
		{ return data; }


	inline String::operator const std::string &( ) const
		{ return data; }


	inline String & String::assign( char ch )
		{ data.assign( 1, ch ); return *this; }


	inline String & String::assign( const char * s )
		{ data.assign( s ); return *this; }


	inline String & String::assign( const Memory & memory )
		{ data.assign( memory.begin( ), memory.end( ) ); return *this; }


	inline String & String::assign( const std::string & str )
		{ data.assign( str ); return *this; }


	inline String & String::assign( const String & str )
		{ data.assign( str.data ); return *this; }


	inline String & String::assign( std::string && str ) noexcept
		{ data.assign( std::move( str ) ); return *this; }


	inline String & String::assign( String && str ) noexcept
		{ data.assign( std::move( str.data ) ); return *this; }


	inline String & String::operator=( char ch )
		{ data.assign( 1, ch ); return *this; }


	inline String & String::operator=( const char * s )
		{ data.assign( s ); return *this; }


	inline String & String::operator=( const Memory & memory )
		{ data.assign( memory.begin( ), memory.end( ) ); return *this; }


	inline String & String::operator=( const std::string & str )
		{ data.assign( str ); return *this; }


	inline String & String::operator=( const String & str )
		{ data.assign( str.data ); return *this; }


	inline String & String::operator=( std::string && str ) noexcept
		{ data.assign( std::move( str ) ); return *this; }


	inline String & String::operator=( String && str ) noexcept
		{ data.assign( std::move( str.data ) ); return *this; }


	inline String & String::append( char ch )
		{ data.append( 1, ch ); return *this; }


	inline String & String::append( size_t count, char ch )
		{ data.append( count, ch ); return *this; }


	inline String & String::append( const char * begin )
		{ data.append( begin ); return *this; }


	inline String & String::append( const char * begin, const char * end )
		{ data.append( begin, end ); return *this; }


	inline String & String::append( const Memory & memory )
		{ data.append( memory.begin( ), memory.end( ) ); return *this; }


	inline String & String::append( const std::string & str )
		{ data.append( str ); return *this; }


	inline String & String::append( const String & str )
		{ data.append( str.data ); return *this; }


	inline String & String::operator+=( char ch )
		{ data.append( 1, ch ); return *this; }


	inline String & String::operator+=( const char * begin )
		{ data.append( begin ); return *this; }


	inline String & String::operator+=( const Memory & memory )
		{ data.append( memory.begin( ), memory.end( ) ); return *this; }


	inline String & String::operator+=( const std::string & str )
		{ data.append( str ); return *this; }


	inline String & String::operator+=( const String & str )
		{ data.append( str.data ); return *this; }


	inline bool String::notEmpty( ) const
		{ return !data.empty( ); }


	inline bool String::isEmpty( ) const
		{ return data.empty( ); }


	inline bool String::operator!( ) const
		{ return isEmpty( ); }


	inline String::operator bool( ) const
		{ return notEmpty( ); }


	inline size_t String::length( ) const
		{ return data.length( ); }


	inline void String::resize( size_t len, char ch )
		{ data.resize( len, ch ); }


	inline void String::clear( )
		{ data.clear( ); }


	inline const char * String::begin( ) const
		{ return data.c_str( ); }


	inline const char * String::end( ) const
		{ return data.c_str( ) + data.length( ); }


	inline char String::at( size_t pos ) const
		{ return data.at( pos ); }


	inline char String::operator[]( size_t pos ) const
		{ return data.at( pos ); }


	inline Memory String::substr( size_t offset, size_t count ) const
		{ return Memory{ data }.substr( offset, count ); }


	inline Memory::Array String::split( const Memory & delimSet, const Memory & trimSet, bool ignoreEmpty ) const
		{ return Memory{ data }.split( delimSet, trimSet, ignoreEmpty ); }


	inline Memory String::trim( const Memory & trimlist ) const
		{ return Memory{ data }.trim( trimlist ); }


	inline size_t String::find( char ch, size_t pos ) const
		{ return Memory{ data }.find( ch, pos ); }


	inline size_t String::find( const Memory & sequence, size_t pos ) const
		{ return Memory{ data }.find( sequence, pos ); }


	inline size_t String::rfind( char ch, size_t pos ) const
		{ return Memory{ data }.rfind( ch, pos ); }


	inline size_t String::rfind( const Memory & sequence, size_t pos ) const
		{ return Memory{ data }.rfind( sequence, pos ); }

	
	inline size_t String::findFirstOf( const Memory & matchset, size_t pos ) const 
		{ return Memory{ data }.findFirstOf( matchset, pos ); }


	inline size_t String::findLastOf( const Memory & matchset, size_t pos ) const
		{ return Memory{ data }.findLastOf( matchset, pos ); }


	inline size_t String::findFirstNotOf( const Memory & matchset, size_t pos ) const
		{ return Memory{ data }.findFirstNotOf( matchset, pos ); }


	inline size_t String::findLastNotOf( const Memory & matchset, size_t pos ) const
		{ return Memory{ data }.findLastNotOf( matchset, pos ); }


	inline Memory::Match String::match( const Memory & regex ) const
		{ return Memory{ data }.match( regex ); }


	inline Memory::Match String::match( const std::regex & regex ) const
		{ return Memory{ data }.match( regex ); }


	inline Memory::Match String::searchOne( const Memory & regex, bool isContinuous ) const
		{ return Memory{ data }.searchOne( regex ); }


	inline Memory::Match String::searchOne( const std::regex & regex, bool isContinuous ) const
		{ return Memory{ data }.searchOne( regex ); }


	inline Memory::Matches String::searchAll( const Memory & regex ) const
		{ return Memory{ data }.searchAll( regex ); }


	inline Memory::Matches String::searchAll( const std::regex & regex ) const
		{ return Memory{ data }.searchAll( regex ); }
	

	inline String & String::replace( const Memory & regex, const Memory & ecmaFormat )
		{ data = Memory{ data }.replace( regex, ecmaFormat ); return *this; }


	inline String & String::replace( const std::regex & regex, const Memory & ecmaFormat )
		{ data = Memory{ data }.replace( regex, ecmaFormat ); return *this; }


	inline bool String::isEscaped( size_t pos ) const
		{ return Memory{ data }.isEscaped( pos ); }


	inline EncodedText String::asText( ) const
		{ return Memory{ data }.asText( ); }


	inline EncodedDecimal String::asDecimal( ) const
		{ return Memory{ data }.asDecimal( ); }


	inline EncodedHex String::asHex( ) const
		{ return Memory{ data }.asHex( ); }


	inline EncodedBase64 String::asBase64( ) const
		{ return Memory{ data }.asBase64( ); }


	inline EncodedBinary String::asBinary( ByteOrder byteOrder ) const
		{ return Memory{ data }.asBinary( byteOrder ); }


	inline String String::format( const Memory & fmt )
		{ return cpp::format( fmt ); }


	template<typename T, typename... Params>
	String String::format( const Memory & fmt, const T & param, Params... parameters )
		{ return cpp::format( fmt, param, parameters... ); }

}


inline bool operator==( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline cpp::String operator+( const cpp::String & lhs, const char * rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const char * lhs, const cpp::String & rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::String{ lhs }.append( rhs.begin(), rhs.end() ); }
inline cpp::String operator+( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const cpp::String & lhs, char ch )
    { return cpp::String{ lhs }.append( 1, ch ); }

