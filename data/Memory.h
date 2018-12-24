#pragma once

/*

	Memory is like a std::span<char> but includes C-style memory operations commonly associated
	with strings.  A Memory object can be thought of as a String that does not own its memory.

	(1) composed pointer to beginning and end of memory range.
	(2) converts to and from std::string and cpp::String
	(3) can emulate C string with single ptr to null-terminated string.  end is unknown until first use of strlen().
	(4) implements traditional string operations (i.e. substr, split, trim, regex, find).
	(5) implements standard C memory operations (i.e. memcmp, memcpy, memmove, byte swap).

*/

#include <string>
#include <vector>

#include <cpp/data/Comparable.h>
#include <cpp/data/ByteOrder.h>
#include <cpp/data/RegexMatch.h>



namespace cpp
{

	class Memory : 
		public Comparable<Memory>, 
		public Comparable<Memory, const std::string &>,
		public Comparable<Memory, const char *>
	{
	public:
									Memory( );
									Memory( const char * cstring );
									Memory( const char * ptr, size_t len );
									Memory( const char * begin, const char * end );
									Memory( const std::string & string );
									Memory( const std::wstring & string );
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

		static int					compare( const Memory & lhs, const char * rhs );
		static int					compare( const Memory & lhs, const std::string & rhs );
		static int					compare( const Memory & lhs, const Memory & rhs );

    private:
		void						ensureEnd( ) const;

	private:
		const char *				m_begin;
		mutable const char *		m_end;
	};

    template<class T>
    T copy( const Memory::Array & src )
        { return T{ src.begin(), src.end() }; }

	Memory::Memory( )
		: m_begin( nullptr ), m_end( nullptr ) { }
	Memory::Memory( const char * cstring )
		: m_begin( cstring ), m_end( nullptr ) { }
	Memory::Memory( const char * ptr, size_t len )
		: m_begin( ptr ), m_end( ptr + len ) { }
	Memory::Memory( const char * begin, const char * end )
		: m_begin( begin ), m_end( end ) { }
	Memory::Memory( const std::string & string )
		: m_begin( string.c_str( ) ), m_end( string.c_str( ) + string.length( ) ) { }
	Memory::Memory( const std::wstring & string )
		: m_begin( (const char *)string.c_str( ) ), m_end( (const char *)string.c_str( ) + string.length( ) * sizeof( wchar_t ) ) { }
	Memory::Memory( const Memory & copy )
		: m_begin( copy.m_begin ), m_end( copy.m_end ) { }

	template<class T, typename>
	Memory Memory::ofValue( const T & value )
	{
		return Memory{ (char *)&value, sizeof( value ) };
	}

	Memory & Memory::operator=( nullptr_t )
	{
		m_begin = nullptr; m_end = nullptr; return *this;
	}
	Memory & Memory::operator=( const char * cstring )
	{
		m_begin = cstring; m_end = nullptr; return *this;
	}
	Memory & Memory::operator=( const std::string & string )
	{
		m_begin = string.c_str( ); m_end = string.c_str( ) + string.length( ); return *this;
	}
	Memory & Memory::operator=( const Memory & memory )
	{
		m_begin = memory.m_begin; m_end = memory.m_end; return *this;
	}

	bool Memory::operator!( ) const
	{
		return isEmpty( );
	}
	Memory::operator bool( ) const
	{
		return !isEmpty( );
	}

	char Memory::at( size_t pos ) const
	{
		return ( pos < length( ) ) ? *( m_begin + pos ) : 0;
	}
	char Memory::operator[]( size_t pos ) const
	{
		return at( pos );
	}

	const char * Memory::data( ) const
	{
		return m_begin;
	}
	const char * Memory::begin( ) const
	{
		return m_begin;
	}
	const char * Memory::end( ) const
	{
		ensureEnd( ); return m_end;
	}

	size_t Memory::length( ) const
	{
		ensureEnd( ); return m_end - m_begin;
	}
	bool Memory::isEmpty( ) const
	{
		return m_end == m_begin || ( !m_end && *m_begin == 0 );
	}
	bool Memory::isNull( ) const
	{
		return begin( ) == nullptr;
	}

	std::string Memory::toString( ) const
	{
		return isNull( ) ? "(null)" : std::string{ m_begin, m_end };
	}

	void Memory::put( size_t pos, char ch )
	{
		*( (char *)m_begin + pos ) = ch;
	}

	template<typename T> T Memory::trySwap( T value, ByteOrder byteOrder )
	{
		return ( byteOrder != ByteOrder::Host ) ? (T)swap( value ) : value;
	}

	int8_t Memory::swap( int8_t value )
	{
		return value;
	}
	uint8_t Memory::swap( uint8_t value )
	{
		return value;
	}
	int16_t Memory::swap( int16_t value )
	{
		return (int16_t)_byteswap_ushort( value );
	}
	uint16_t Memory::swap( uint16_t value )
	{
		return _byteswap_ushort( value );
	}
	int32_t Memory::swap( int32_t value )
	{
		return (int32_t)_byteswap_ulong( value );
	}
	uint32_t Memory::swap( uint32_t value )
	{
		return _byteswap_ulong( value );
	}
	int64_t Memory::swap( int64_t value )
	{
		return (int64_t)_byteswap_uint64( value );
	}
	uint64_t Memory::swap( uint64_t value )
	{
		return _byteswap_uint64( value );
	}

	void Memory::ensureEnd( ) const
	{
		if ( !m_end && m_begin ) { m_end = m_begin + strlen( m_begin ); }
	}

	int	Memory::compare( const Memory & lhs, const char * rhs )
	{
        if ( lhs.isNull( ) )
            { return ( rhs == nullptr ) ? 0 : -1; }
        if ( rhs == nullptr )
            { return 1; }

        if ( !lhs.m_end )
            { return strcmp( lhs.data( ), rhs ); }

        int result = memcmp( lhs.data( ), rhs, lhs.length( ) );
		if ( result == 0 && rhs[lhs.length( )] != 0 )
			{ return -1; }
        return result;
	}

	int	Memory::compare( const Memory & lhs, const std::string & rhs )
	{
		if ( lhs.isNull( ) )
            { return -1; }
    
		if ( !lhs.m_end )
            { return strcmp( lhs.data( ), rhs.data( ) ); }

		size_t len1 = lhs.length( );
        size_t len2 = rhs.length( );
        int result = memcmp( lhs.data( ), rhs.data( ), std::min( len1, len2 ) );
		if ( result == 0 && len1 != len2 )
			{ return ( len1 < len2 ) ? -1 : 1; }
        return result;
	}

	int	Memory::compare( const Memory & lhs, const Memory & rhs )
	{
		if ( lhs.isNull( ) )
            { return rhs.isNull( ) ? 0 : -1; }
        if ( rhs.isNull( ) )
            { return 1; }
    
		if ( !rhs.m_end )
			{ return strcmp( lhs.data( ), rhs.data( ) ); }
		if ( !lhs.m_end )
            { return strcmp( lhs.data( ), rhs.data( ) ); }

		size_t len1 = lhs.length( );
        size_t len2 = rhs.length( );
        int result = memcmp( lhs.data( ), rhs.data( ), std::min( len1, len2 ) );
		if ( result == 0 && len1 != len2 )
			{ return ( len1 < len2 ) ? -1 : 1; }
        return result;
	}



    inline bool operator==( const char * lhs, Memory rhs )
        { return Memory{ lhs } == rhs; }
    inline bool operator==( const std::string & lhs, Memory rhs )
        { return Memory{ lhs } == rhs; }

    inline bool operator!=( const char * lhs, Memory rhs )
        { return Memory{ lhs } != rhs; }

    inline bool operator<( const char * lhs, Memory rhs )
        { return Memory{ lhs } < rhs; }

    inline bool operator<=( const char * lhs, Memory rhs )
        { return Memory{ lhs } <= rhs; }

    inline bool operator>( const char * lhs, Memory rhs )
        { return Memory{ lhs } > rhs; }

    inline bool operator>=( const char * lhs, Memory rhs )
        { return Memory{ lhs } >= rhs; }

}
