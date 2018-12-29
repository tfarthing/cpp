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

#include "ByteOrder.h"
#include "RegexMatch.h"



namespace cpp
{

	struct EncodedText;
	struct EncodedDecimal;
	struct EncodedHex;
	struct EncodedBase64;
	struct EncodedBinary;



	class Memory
	{
	public:
									Memory( );
									Memory( const char * cstring );
									Memory( const char * ptr, size_t len );
									Memory( const char * begin, const char * end );
									Memory( const std::string & string );
									Memory( const Memory & copy );

		Memory &					operator=( nullptr_t );
		Memory &					operator=( const char * cstring );
		Memory &					operator=( const std::string & string );
		Memory &					operator=( const Memory & memory );

		static const size_t			npos = (size_t)-1;
		typedef std::vector<Memory> Array;
		static const Memory			WhitespaceList;

		static Memory				copy( Memory & dst, const Memory & src );		// memcpy
		static Memory				move( Memory & dst, const Memory & src );		// memmove

		Memory						substr( size_t pos = 0, size_t len = npos ) const;

		Memory::Array				split( Memory delimiter, Memory trimlist = WhitespaceList, bool ignoreEmpty = true ) const;
		Memory						trim( Memory trimlist = WhitespaceList ) const;

		const char *				data( ) const;
		const char *				begin( ) const;
		const char *				end( ) const;

		size_t						length( ) const;

		char						at( size_t pos ) const;
		char						operator[]( size_t pos ) const;

		bool						operator!( ) const;				 // isEmpty()
									explicit operator bool( ) const; // notEmpty()
		bool						isEmpty( ) const;			     // isNull() or empty()
		bool						notEmpty( ) const;				 // notNull() and !empty()
		bool						isNull( ) const;
		bool						notNull( ) const;

		typedef RegexMatch<Memory>	Match;
		typedef std::vector<Match>	Matches;
		Match						match( Memory regex ) const;
        Match						match( const std::regex & regex ) const;
        Match						searchOne( Memory regex, bool isContinuous = false ) const;
        Match						searchOne( const std::regex & regex, bool isContinuous = false ) const;
        Matches						searchAll( Memory regex ) const;
        Matches						searchAll( const std::regex & regex ) const;
        std::string					replace( Memory regex, Memory ecmaFormat ) const;
        std::string					replace( const std::regex & regex, Memory ecmaFormat ) const;

		size_t						find( char ch, size_t pos = 0 ) const;
		size_t						find( Memory sequence, size_t pos = 0 ) const;
		size_t						rfind( char ch, size_t pos = npos ) const;
		size_t						rfind( Memory sequence, size_t pos = npos ) const;
		size_t						find_first_of( Memory matchset, size_t pos = 0 ) const;
		size_t						find_last_of( Memory matchset, size_t pos = npos ) const;
		size_t						find_first_not_of( Memory matchset, size_t pos = 0 ) const;
		size_t						find_last_not_of( Memory matchset, size_t pos = npos ) const;

		std::string					toString( ) const;

		bool						isEscaped( size_t pos ) const;
		void						put( size_t pos, char ch );
        
		template<class T, typename = std::enable_if_t<std::is_scalar<T>::value>>
		static Memory				ofValue( const T & value );

		static int8_t				swap( int8_t value );
		static uint8_t				swap( uint8_t value );
		static int16_t				swap( int16_t value );
		static uint16_t				swap( uint16_t value );
		static int32_t				swap( int32_t value );
		static uint32_t				swap( uint32_t value );
		static int64_t				swap( int64_t value );
		static uint64_t				swap( uint64_t value );
        static float				swap( float value );
        static double				swap( double value );

		template<typename T>
		static T					trySwap( T value, ByteOrder byteOrder );

		static int					compare( const Memory & lhs, const Memory & rhs );

		EncodedText					asText( ) const;
		EncodedDecimal				asDecimal( ) const;
		EncodedHex					asHex( ) const;
		EncodedBase64				asBase64( ) const;
		EncodedBinary				asBinary( ByteOrder byteOrder = ByteOrder::Host ) const;

    private:
		void						ensureEnd( ) const;

	private:
		const char *				m_begin;
		mutable const char *		m_end;
	};


    template<class T>
    T copy( const Memory::Array & src )
        { return T{ src.begin(), src.end() }; }

	inline Memory::Memory( )
		: m_begin( nullptr ), m_end( nullptr ) { }
	
	inline Memory::Memory( const char * cstring )
		: m_begin( cstring ), m_end( nullptr ) { }
	
	inline Memory::Memory( const char * ptr, size_t len )
		: m_begin( ptr ), m_end( ptr + len ) { }
	
	inline Memory::Memory( const char * begin, const char * end )
		: m_begin( begin ), m_end( end ) { }
	
	inline Memory::Memory( const std::string & string )
		: m_begin( string.c_str( ) ), m_end( string.c_str( ) + string.length( ) ) { }
	
	inline Memory::Memory( const Memory & copy )
		: m_begin( copy.m_begin ), m_end( copy.m_end ) { }

	template<class T, typename>
	Memory Memory::ofValue( const T & value )
		{ return Memory{ (char *)&value, sizeof( value ) }; }

	inline Memory & Memory::operator=( nullptr_t )
		{ m_begin = nullptr; m_end = nullptr; return *this; }

	inline Memory & Memory::operator=( const char * cstring )
		{ m_begin = cstring; m_end = nullptr; return *this; }

	inline Memory & Memory::operator=( const std::string & string )
		{ m_begin = string.c_str( ); m_end = string.c_str( ) + string.length( ); return *this; }

	inline Memory & Memory::operator=( const Memory & memory )
		{ m_begin = memory.m_begin; m_end = memory.m_end; return *this; }

	inline bool Memory::operator!( ) const
		{ return isEmpty( ); }

	inline Memory::operator bool( ) const
		{ return notEmpty( ); }

	inline char Memory::at( size_t pos ) const
		{ return ( pos < length( ) ) ? *( m_begin + pos ) : 0; }

	inline char Memory::operator[]( size_t pos ) const
		{ return at( pos ); }

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
		{ return isNull( ) ? "(null)" : std::string{ begin( ), end( ) }; }

	inline void Memory::put( size_t pos, char ch )
		{ *( (char *)begin( ) + pos ) = ch; }


	template<typename T> T Memory::trySwap( T value, ByteOrder byteOrder )
		{ return ( byteOrder != ByteOrder::Host ) ? (T)swap( value ) : value; }

	inline int8_t Memory::swap( int8_t value )
		{ return value; }

	inline uint8_t Memory::swap( uint8_t value )
		{ return value; }

	inline int16_t Memory::swap( int16_t value )
		{ return (int16_t)_byteswap_ushort( value ); }

	inline uint16_t Memory::swap( uint16_t value )
		{ return _byteswap_ushort( value ); }

	inline int32_t Memory::swap( int32_t value )
		{ return (int32_t)_byteswap_ulong( value ); }

	inline uint32_t Memory::swap( uint32_t value )
		{ return _byteswap_ulong( value ); }

	inline int64_t Memory::swap( int64_t value )
		{ return (int64_t)_byteswap_uint64( value ); }

	inline uint64_t Memory::swap( uint64_t value )
		{ return _byteswap_uint64( value ); }

	inline void Memory::ensureEnd( ) const
		{ if ( !end( ) && begin( ) ) { m_end = begin( ) + strlen( begin( ) ); } }



	struct EncodedMemory
	{
		EncodedMemory( Memory data_ )
			: data( data_ ) { }
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
		operator std::string( ) const;
	};



	struct EncodedBase64 : public EncodedMemory
	{
		using EncodedMemory::EncodedMemory;
		operator uint8_t( ) const;
		operator uint16_t( ) const;
		operator uint32_t( ) const;
		operator uint64_t( ) const;
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
