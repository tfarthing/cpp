#pragma once

/*

	Memory is like a std::span<char> but includes C-style memory operations commonly associated
	with strings.  A Memory object can be thought of as a String that does not own its data.

	(1) composed pointer to beginning and end of memory range.
	(2) converts to and from std::string and cpp::String
	(3) can emulate C string with single ptr to null-terminated string.  end is null until first use of strlen().
	(4) implements traditional string operations (i.e. substr, split, trim, regex, find).
	(5) implements standard C memory operations (i.e. memcmp, memcpy, memmove, byte swap).
	(6) implements decoder methods (asText, asDecimal, asHex, asBase64, and asBinary).

*/

#include <string>
#include <vector>

#include "Primitive.h"
#include "ByteOrder.h"
#include "RegexMatch.h"



namespace cpp
{

	struct EncodedText;
	struct EncodedDecimal;
	struct EncodedHex;
	struct EncodedBase64;
	struct EncodedBinary;
	struct String;
	class Memory;



	class Memory
	{
	public:
											Memory( );
                                            Memory( nullptr_t );
											Memory( const char * cstring );
											Memory( const char * ptr, size_t len );
											Memory( const char * begin, const char * end );
        explicit                            Memory( const char8_t * cstring );
                                            Memory( const std::string & string );
                                            Memory( const String & copy );
											Memory( const Memory & copy );

		Memory &							operator=( const Memory & memory );

		bool								operator<( const Memory & memory ) const;

		static const size_t					npos = (size_t)-1;
		typedef std::vector<Memory>			Array;
		static const Memory					Empty;
		static const Memory					WhitespaceList;

		bool								isEmpty( ) const;			     // isNull() or empty()
		bool								notEmpty( ) const;				 // notNull() and !empty()
		bool								isNull( ) const;
		bool								notNull( ) const;
		bool								operator!( ) const;				 // isEmpty()
											explicit operator bool( ) const; // notEmpty()

		size_t								length( ) const;

		char *								data( );
        const char *						data( ) const;
		const char *						begin( ) const;
		const char *						end( ) const;

		char								at( size_t pos ) const;
		char								operator[]( size_t pos ) const;

		Memory								substr( size_t pos = 0, size_t len = npos ) const;
		Memory::Array						split( const Memory & delimiter, const Memory & trimlist = WhitespaceList, bool ignoreEmpty = true ) const;
		Memory								trim( const Memory & trimlist = WhitespaceList ) const;
        Memory								trimFront( const Memory & trimlist = WhitespaceList ) const;
        Memory								trimBack( const Memory & trimlist = WhitespaceList ) const;

		size_t								find( char ch, size_t pos = 0 ) const;
		size_t								find( const Memory & sequence, size_t pos = 0 ) const;
		size_t								rfind( char ch, size_t pos = npos ) const;
		size_t								rfind( const Memory & sequence, size_t pos = npos ) const;
		size_t								findFirstOf( const Memory & matchset, size_t pos = 0 ) const;
		size_t								findLastOf( const Memory & matchset, size_t pos = npos ) const;
		size_t								findFirstNotOf( const Memory & matchset, size_t pos = 0 ) const;
		size_t								findLastNotOf( const Memory & matchset, size_t pos = npos ) const;

		std::string							replaceFirst( const Memory & sequence, const Memory & dst, size_t pos = 0 ) const;
		std::string							replaceLast( const Memory & sequence, const Memory & dst, size_t pos = npos ) const;
		std::string							replaceAll( const Memory & sequence, const Memory & dst, size_t pos = 0 ) const;

		typedef RegexMatch<Memory>			Match;
		typedef std::vector<Match>			Matches;
		Match								match( const Memory & regex ) const;
		Match								match( const std::regex & regex ) const;
		Match								searchOne( const Memory & regex, bool isContinuous = false ) const;
		Match								searchOne( const std::regex & regex, bool isContinuous = false ) const;
		Matches								searchAll( const Memory & regex ) const;
		Matches								searchAll( const std::regex & regex ) const;
		std::string							replace( const Memory & regex, const Memory & ecmaFormat ) const;
		std::string							replace( const std::regex & regex, const Memory & ecmaFormat ) const;

		std::string							toString( ) const;
											operator std::string( ) const;

		bool								endsWith( const Memory & sequence ) const;
		bool								isEscaped( size_t pos ) const;
		void								poke( size_t pos, char ch );

		EncodedText							asText( ) const;
		EncodedDecimal						asDecimal( ) const;
		EncodedHex							asHex( ) const;
		EncodedBase64						asBase64( ) const;
		EncodedBinary						asBinary( ByteOrder byteOrder = ByteOrder::Host ) const;

		Memory								printf( const char * fmt, ... );

		Memory								format( const Memory & fmt );
		template<typename T, typename... Params>
		Memory								format( const Memory & fmt, const T & param, Params... parameters );
		template<typename T, typename... Params>
		Memory								format( size_t pos, const Memory & fmt, const T & param, Params... parameters );
		Memory								format( size_t pos, const Memory & fmt );

		static int							compare( const Memory & lhs, const Memory & rhs );	// memcmp
		static Memory						copy( Memory dst, const Memory & src );			// memcpy
		static Memory						move( Memory dst, const Memory & src );			// memmove

		template<class T, typename = std::enable_if_t<std::is_scalar<T>::value>>
		static Memory						ofValue( const T & value );

		static int8_t						byteswap( int8_t value );
		static uint8_t						byteswap( uint8_t value );
		static int16_t						byteswap( int16_t value );
		static uint16_t						byteswap( uint16_t value );
		static int32_t						byteswap( int32_t value );
		static uint32_t						byteswap( uint32_t value );
		static int64_t						byteswap( int64_t value );
		static uint64_t						byteswap( uint64_t value );
        static float						byteswap( float value );
        static double						byteswap( double value );

		template<typename T>
		static T							tryByteSwap( T value, ByteOrder byteOrder );

    private:
		void								ensureEnd( ) const;

	private:
		const char *						m_begin;
		mutable const char *				m_end;
	};



	inline std::string toString( bool value )
        { return value ? "true" : "false"; }


    inline std::string toString( int64_t value )
    {
        char buffer[32];
        snprintf( buffer, sizeof( buffer ), "%lli", value );
        return buffer;
    }


    inline std::string toString( uint64_t value )
    {
        char buffer[32];
        snprintf( buffer, sizeof( buffer ), "%llu", value );
        return buffer;
    }


    inline std::string toString( int32_t value )
        { return toString( (int64_t)value ); }


    inline std::string toString( uint32_t value )
        { return toString( (uint64_t)value ); }


    inline std::string toString( int16_t value )
        { return toString( (int64_t)value ); }


    inline std::string toString( uint16_t value )
        { return toString( (uint64_t)value ); }


    inline std::string toString( int8_t value )
        { return toString( (int64_t)value ); }


    inline std::string toString( uint8_t value )
        { return toString( (uint64_t)value ); }


    template<typename T> std::string toString( const T & value )
        { return value.toString( ); }


    inline std::string toString( const Memory::Array & array )
    { 
        std::string result; 
        for ( auto & element : array )
        {
            if ( !result.empty( ) )
                { result += ", "; }
            result += element;
        }
        return result;
    }


	std::string toString( const String & str );
	std::string toString( const std::vector<String> & array );


    inline std::string toString( double value )
    {
        char buffer[32];
        int len = snprintf( buffer, sizeof( buffer ), "%f", value );
        while ( len > 0 && buffer[len - 1] == '0' )
            { buffer[--len] = 0; }
        return std::string{ buffer };
    }


    inline std::string toString( float value )
        { return toString( (double)value ); }


    inline std::string toString( const char * value )
        { return std::string{ value }; }


    inline std::string toString( const std::string & value )
        { return value; }


    inline std::string toString( Memory & value )
        { return std::string{ value }; }


    inline std::string toString( const Memory & value )
        { return std::string{ value }; }



    template<class T>
    T copy( const Memory::Array & src )
        { return T{ src.begin(), src.end() }; }


	inline Memory::Memory( )
		: m_begin( nullptr ), m_end( nullptr ) { }
	
    inline Memory::Memory( nullptr_t )
        : m_begin( nullptr ), m_end( nullptr ) { }

	inline Memory::Memory( const char * cstring )
		: m_begin( cstring ), m_end( nullptr ) { }
	

	inline Memory::Memory( const char * ptr, size_t len )
		: m_begin( ptr ), m_end( ptr + len ) { }
	

	inline Memory::Memory( const char * begin, const char * end )
		: m_begin( begin ), m_end( end ) { }
	
    inline Memory::Memory( const char8_t * cstring )
        : m_begin( (char *)cstring ), m_end( nullptr ) { }

	inline Memory::Memory( const std::string & string )
		: m_begin( string.c_str( ) ), m_end( string.c_str( ) + string.length( ) ) { }
	

	inline Memory::Memory( const Memory & copy )
		: m_begin( copy.m_begin ), m_end( copy.m_end ) { }


	inline Memory Memory::format( const Memory & fmt )
	{
		return format( 0, fmt );
	}


	template<typename T, typename... Params>
	Memory Memory::format( const Memory & fmt, const T & param, Params... parameters )
	{
		return format( 0, fmt, param, parameters... );
	}


	template<typename T, typename... Params>
	Memory Memory::format( size_t pos, const Memory & fmt, const T & param, Params... parameters )
	{
		size_t fpos = fmt.find( '%' );
		if ( fpos == Memory::npos )
		{
			return format( pos, fmt );
		}
		else
		{
			size_t end = pos;
			end += format( end, fmt.substr( 0, fpos ) ).length( );
			end += format( end, cpp::toString( param ) ).length( );
			end += format( end, fmt.substr( fpos + 1 ), parameters... ).length();
			return substr( pos, end - pos );
		}
	}


	template<class T, typename>
	Memory Memory::ofValue( const T & value )
		{ return Memory{ (char *)&value, sizeof( value ) }; }


    /*
	inline Memory & Memory::operator=( nullptr_t )
		{ m_begin = nullptr; m_end = nullptr; return *this; }


	inline Memory & Memory::operator=( const char * cstring )
		{ m_begin = cstring; m_end = nullptr; return *this; }


	inline Memory & Memory::operator=( const std::string & string )
		{ m_begin = string.c_str( ); m_end = string.c_str( ) + string.length( ); return *this; }
    */

	inline Memory & Memory::operator=( const Memory & memory )
		{ m_begin = memory.m_begin; m_end = memory.m_end; return *this; }


	inline bool Memory::operator<( const Memory & memory ) const
		{ return compare( *this, memory ) < 0; }


	inline bool Memory::operator!( ) const
		{ return isEmpty( ); }


	inline Memory::operator bool( ) const
		{ return notEmpty( ); }


	inline char Memory::at( size_t pos ) const
		{ return ( pos < length( ) ) ? *( m_begin + pos ) : 0; }


	inline char Memory::operator[]( size_t pos ) const
		{ return at( pos ); }


	inline char * Memory::data( )
		{ return (char *)m_begin; }


	inline const char * Memory::data( ) const
		{ return m_begin; }


	inline const char * Memory::begin( ) const
		{ return m_begin; }


	inline const char * Memory::end( ) const
		{ ensureEnd( ); return m_end; }


	inline size_t Memory::length( ) const
		{ ensureEnd( ); return end( ) - begin( ); }


	inline bool Memory::isEmpty( ) const
		{ return begin( ) == nullptr || end( ) == begin( ); }


	inline bool Memory::notEmpty( ) const
		{ return begin( ) != nullptr && end( ) != begin( ); }


	inline bool Memory::isNull( ) const
		{ return begin( ) == nullptr; }


	inline bool Memory::notNull( ) const
		{ return begin( ) != nullptr; }


	inline std::string Memory::toString( ) const
		{ return isNull( ) ? "]null[" : std::string{ begin( ), end( ) }; }
	

	inline Memory::operator std::string( ) const
		{ return toString(); }


	inline void Memory::poke( size_t pos, char ch )
		{ *( (char *)begin( ) + pos ) = ch; }


	template<typename T> T Memory::tryByteSwap( T value, ByteOrder byteOrder )
		{ return ( byteOrder != ByteOrder::Host ) ? (T)byteswap( value ) : value; }


	inline int8_t Memory::byteswap( int8_t value )
		{ return value; }


	inline uint8_t Memory::byteswap( uint8_t value )
		{ return value; }


	inline int16_t Memory::byteswap( int16_t value )
		{ return (int16_t)_byteswap_ushort( value ); }


	inline uint16_t Memory::byteswap( uint16_t value )
		{ return _byteswap_ushort( value ); }


	inline int32_t Memory::byteswap( int32_t value )
		{ return (int32_t)_byteswap_ulong( value ); }


	inline uint32_t Memory::byteswap( uint32_t value )
		{ return _byteswap_ulong( value ); }


	inline int64_t Memory::byteswap( int64_t value )
		{ return (int64_t)_byteswap_uint64( value ); }


	inline uint64_t Memory::byteswap( uint64_t value )
		{ return _byteswap_uint64( value ); }


	inline void Memory::ensureEnd( ) const
		{ if ( !m_end && m_begin ) { m_end = m_begin + strlen( m_begin ); } }



	struct EncodedMemory
	{
		EncodedMemory( Memory data_ )
			: data( data_ ) { }
        operator Memory( ) const
            { return data; }
		Memory data;
	};



	struct EncodedText : public EncodedMemory
	{
		using EncodedMemory::EncodedMemory;
		operator int8_t( ) const;
		operator uint8_t( ) const;
		operator int16_t( ) const;
		operator uint16_t( ) const;
		operator int32_t( ) const;
		operator uint32_t( ) const;
		operator int64_t( ) const;
		operator uint64_t( ) const;
		operator float( ) const;
		operator double( ) const;
		operator bool( ) const;
	};



	struct EncodedDecimal : public EncodedMemory
	{
		using EncodedMemory::EncodedMemory;
		operator int8_t( ) const;
		operator uint8_t( ) const;
		operator int16_t( ) const;
		operator uint16_t( ) const;
		operator int32_t( ) const;
		operator uint32_t( ) const;
		operator int64_t( ) const;
		operator uint64_t( ) const;
		operator float( ) const;
		operator double( ) const;
	};



	struct EncodedHex : public EncodedMemory
	{
		using EncodedMemory::EncodedMemory;
        operator uint8_t( ) const;
        operator uint16_t( ) const;
        operator uint32_t( ) const;
        operator uint64_t( ) const;
        operator int8_t( ) const;
		operator int16_t( ) const;
		operator int32_t( ) const;
		operator int64_t( ) const;
		operator std::string( ) const;
	};



	struct EncodedBase64 : public EncodedMemory
	{
		using EncodedMemory::EncodedMemory;
		operator std::string( ) const;
	};



	struct EncodedBinary : public EncodedMemory
	{
		EncodedBinary( Memory data_, ByteOrder byteOrder_ )
			: EncodedMemory( data_ ), byteOrder( byteOrder_ ) { };

		operator int8_t( ) const;
		operator uint8_t( ) const;
		operator int16_t( ) const;
		operator uint16_t( ) const;
		operator int32_t( ) const;
		operator uint32_t( ) const;
		operator int64_t( ) const;
		operator uint64_t( ) const;
		operator float( ) const;
		operator double( ) const;
		operator bool( ) const;

		ByteOrder byteOrder;
	};



	inline EncodedText Memory::asText( ) const
		{ return EncodedText{ *this }; }

	inline EncodedDecimal Memory::asDecimal( ) const
		{ return EncodedDecimal{ *this }; }

	inline EncodedHex Memory::asHex( ) const
		{ return EncodedHex{ *this }; }

	inline EncodedBase64 Memory::asBase64( ) const
		{ return EncodedBase64{ *this }; }

	inline EncodedBinary Memory::asBinary( ByteOrder byteOrder ) const
		{ return EncodedBinary{ *this, byteOrder }; }

}


inline bool operator==( const cpp::Memory & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::Memory & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::Memory & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::Memory & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::Memory & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::Memory & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const cpp::Memory & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::Memory & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::Memory & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::Memory & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::Memory & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::Memory & lhs, const char * rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const char * lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const char * lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const char * lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const char * lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const char * lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const char * lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const std::string & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const std::string & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const std::string & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const std::string & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const std::string & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const std::string & lhs, const cpp::Memory & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline bool operator==( const cpp::Memory & lhs, const std::string & rhs )
    { return cpp::Memory::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::Memory & lhs, const std::string & rhs )
    { return cpp::Memory::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::Memory & lhs, const std::string & rhs )
    { return cpp::Memory::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::Memory & lhs, const std::string & rhs )
    { return cpp::Memory::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::Memory & lhs, const std::string & rhs )
    { return cpp::Memory::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::Memory & lhs, const std::string & rhs )
    { return cpp::Memory::compare( lhs, rhs ) >= 0; }


inline std::string operator+( const cpp::Memory & lhs, const char * rhs )
    { return std::string{ lhs }.append( rhs ); }
inline std::string operator+( const char * lhs, const cpp::Memory & rhs )
    { return std::string{ lhs }.append( rhs ); }
inline std::string operator+( const std::string & lhs, const cpp::Memory & rhs )
    { return std::string{ lhs }.append( rhs.begin(), rhs.end() ); }
inline std::string operator+( const cpp::Memory & lhs, const std::string & rhs )
    { return std::string{ lhs }.append( rhs ); }

