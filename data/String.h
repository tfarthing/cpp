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

	class String;

	template<typename T, typename... Params>
	std::string format( const Memory & fmt, const T & param, Params... parameters );
	std::string format( const Memory & fmt );



    class String
		: public std::string
	{
	public:
		String( );
		String( size_type count, value_type ch, const allocator_type & alloc = allocator_type( ) );
		String( const value_type * s, const allocator_type & alloc = allocator_type( ) );
		String( const value_type * s, size_type count, const allocator_type & alloc = allocator_type( ) );
		template< class InputIt > String( InputIt first, InputIt last, const allocator_type & alloc = allocator_type( ) );
		String( const Memory & memory );
		String( const std::string & copy );
		String( const String & copy );
		String( std::string && move );
		String( String && move );

        typedef std::vector<String> Array;

		String & operator=( const Memory & memory );
		String & operator=( const std::string & str );
		String & operator=( const String & str );
		String & operator=( std::string && str );
		String & operator=( String && str );
		String & operator=( const value_type * s );
		String & operator=( value_type ch );

		String & append( const char * begin );
		String & append( const char * begin, const char * end );
		String & append( const Memory & memory );
		String & append( char ch );
		String & append( size_t count, char ch );

		String & operator+=( const char * begin );
		String & operator+=( const Memory & memory );
		String & operator+=( char ch );

		bool isEmpty( ) const;

		Memory substr( size_t offset, size_t count = npos ) const;
		Memory::Array split( const Memory &  delimSet, const Memory & trimSet = Memory::WhitespaceList, bool ignoreEmpty = true ) const;
		Memory trim( const Memory & trimSet = Memory::WhitespaceList ) const;

        String & replaceFirst( const Memory & sequence, const Memory & dst, size_t pos = 0 );
        String & replaceLast( const Memory & sequence, const Memory & dst, size_t pos = npos );
		String & replaceAll( const Memory & sequence, const Memory & dst, size_t pos = 0 );

		Memory::Match match( Memory regex ) const;
		Memory::Match match( const std::regex & regex ) const;
		Memory::Match searchOne( Memory regex, bool isContinuous = false ) const;
		Memory::Match searchOne( const std::regex & regex, bool isContinuous = false ) const;
		Memory::Matches searchAll( Memory regex ) const;
		Memory::Matches searchAll( const std::regex & regex ) const;
		String replaceRegex( Memory regex, Memory ecmaFormat ) const;
		String replaceRegex( const std::regex & regex, Memory ecmaFormat ) const;

		String & toUpper( );
		String & toLower( );

		String toString( ) const;

		static String printf( const char * fmt, ... );

		template<typename T, typename... Params>
		static String format( const Memory & fmt, const T & param, Params... parameters );
		static String format( const Memory & fmt );
	};


	inline String::String( )
		: std::string( ) { }

	inline String::String( size_type count, value_type ch, const allocator_type & alloc )
		: std::string( count, ch, alloc ) { }

	inline String::String( const value_type * s, const allocator_type & alloc )
		: std::string( s, alloc ) { }

	inline String::String( const value_type * s, size_type count, const allocator_type & alloc )
		: std::string( s, count, alloc ) { }

	template< class InputIt > String::String( InputIt first, InputIt last, const allocator_type & alloc )
		: std::string( first, last, alloc ) { }

	inline String::String( const Memory & memory )
		: std::string( memory.begin( ), memory.begin( ) + memory.length( ) ) { }

	inline String::String( const std::string & copy )
		: std::string( copy ) { }

	inline String::String( const String & copy )
		: std::string( copy ) { }

	inline String::String( std::string && move )
		: std::string( std::move( move ) ) { }

	inline String::String( String && move )
		: std::string( std::move( move ) ) { }

	inline String & String::operator=( const Memory & memory )
		{ std::string::assign( memory.begin( ), memory.end( ) ); return *this; }

	inline String & String::operator=( const std::string & str )
		{ std::string::operator=( str ); return *this; }

	inline String & String::operator=( const String & str )
		{ std::string::operator=( str ); return *this; }

	inline String & String::operator=( std::string && str )
		{ std::string::operator=( std::move( str ) ); return *this; }

	inline String & String::operator=( String && str )
		{ std::string::operator=( std::move( str ) ); return *this; }

	inline String & String::operator=( const value_type * s )
		{ std::string::operator=( s ); return *this; }

	inline String & String::operator=( value_type ch )
		{ std::string::operator=( ch ); return *this; }

	inline String & String::append( const char * begin )
		{ std::string::append( begin ); return *this; }

	inline String & String::append( const char * begin, const char * end )
		{ std::string::append( begin, end ); return *this; }

	inline String & String::append( const Memory & memory )
		{ std::string::append( memory.begin( ), memory.end( ) ); return *this; }

	inline String & String::append( char ch )
		{ std::string::append( 1, ch ); return *this; }

	inline String & String::append( size_t count, char ch )
		{ std::string::append( count, ch ); return *this; }

	inline String & String::operator+=( const char * begin )
		{ std::string::append( begin ); return *this; }

	inline String & String::operator+=( const Memory & memory )
		{ std::string::append( memory.begin( ), memory.end( ) ); return *this; }

	inline String & String::operator+=( char ch )
		{ std::string::append( 1, ch ); return *this; }

	inline bool String::isEmpty( ) const
		{ return empty( ); }

	inline Memory String::substr( size_t offset, size_t count ) const
		{ return Memory{ *this }.substr( offset, count ); }

	inline Memory String::trim( const Memory & trimSet ) const
		{ return Memory{ *this }.trim( trimSet ); }

	inline Memory::Array String::split( const Memory &  delimSet, const Memory & trimSet, bool ignoreEmpty ) const
		{ return Memory{ *this }.split( delimSet, trimSet, ignoreEmpty ); }

	inline Memory::Match String::match( Memory regex ) const
		{ return Memory{ *this }.match( regex ); }

	inline Memory::Match String::match( const std::regex & regex ) const
		{ return Memory{ *this }.match( regex ); }

	inline Memory::Match String::searchOne( Memory regex, bool isContinuous ) const
		{ return Memory{ *this }.searchOne( regex ); }

	inline Memory::Match String::searchOne( const std::regex & regex, bool isContinuous ) const
		{ return Memory{ *this }.searchOne( regex ); }

	inline Memory::Matches String::searchAll( Memory regex ) const
		{ return Memory{ *this }.searchAll( regex ); }

	inline Memory::Matches String::searchAll( const std::regex & regex ) const
		{ return Memory{ *this }.searchAll( regex ); }
	
	inline String String::replaceRegex( Memory regex, Memory ecmaFormat ) const
		{ return Memory{ *this }.replace( regex, ecmaFormat ); }

	inline String String::replaceRegex( const std::regex & regex, Memory ecmaFormat ) const
		{ return Memory{ *this }.replace( regex, ecmaFormat ); }

	inline String String::toString( ) const
		{ return *this; }


	inline String String::format( const Memory & fmt )
		{ return fmt; }

	template<typename T, typename... Params>
	String String::format( const Memory & fmt, const T & param, Params... parameters )
	{
		String result;

		size_t begin = 0;
		size_t pos = 0;
		while ( pos < fmt.length( ) )
		{
			if ( fmt[pos] == '%' )
			{
				return fmt.substr( begin, pos - begin ) +
					cpp::toString( param ) +
					format( fmt.substr( pos + 1 ), parameters... );
			}
			pos++;
		}
		result += fmt.substr( begin, pos - begin );
		return result;
	}



    inline String toString( bool value )
        { return value ? "true" : "false"; }

    inline String toString( int64_t value )
    {
        char buffer[32];
        snprintf( buffer, sizeof( buffer ), "%lli", value );
        return String{ buffer };
    }

    inline String toString( uint64_t value )
    {
        char buffer[32];
        snprintf( buffer, sizeof( buffer ), "%llu", value );
        return String{ buffer };
    }

    inline String toString( int32_t value )
        { return toString( (int64_t)value ); }

    inline String toString( uint32_t value )
        { return toString( (uint64_t)value ); }

    inline String toString( int16_t value )
        { return toString( (int64_t)value ); }

    inline String toString( uint16_t value )
        { return toString( (uint64_t)value ); }

    inline String toString( int8_t value )
        { return toString( (int64_t)value ); }

    inline String toString( uint8_t value )
        { return toString( (uint64_t)value ); }

    template<typename T> String toString( const T & value )
        { return value.toString( ); }

    inline String toString( const Memory::Array & array )
    { 
        String result; 
        for ( auto & element : array )
        {
            if ( !result.isEmpty() )
                { result += ", "; }
            result += element;
        }
        return result;
    }

    inline String toString( const String::Array & array )
    { 
        String result; 
        for ( auto & element : array )
        {
            if ( !result.isEmpty() )
                { result += ", "; }
            result += element;
        }
        return result;
    }

    inline String toString( double value )
    {
        char buffer[32];
        int len = snprintf( buffer, sizeof( buffer ), "%f", value );
        while ( len > 0 && buffer[len - 1] == '0' )
            { buffer[--len] = 0; }
        return String{ buffer };
    }

    inline String toString( float value )
        { return toString( (double)value ); }

    inline String toString( const char * value )
        { return String{ value }; }

    inline String toString( const std::string & value )
        { return String{ value }; }

    inline String toString( const Memory & value )
        { return String{ value }; }

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
inline cpp::String operator+( const cpp::Memory & lhs, const char * rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const char * lhs, const cpp::String & rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const char * lhs, const cpp::Memory & rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::String{ lhs }.append( rhs.begin(), rhs.end() ); }
inline cpp::String operator+( const cpp::Memory & lhs, const cpp::String & rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::String{ lhs }.append( rhs ); }
inline cpp::String operator+( const cpp::String & lhs, char ch )
    { return cpp::String{ lhs }.append( 1, ch ); }

