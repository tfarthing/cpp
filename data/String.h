#pragma once

#include <string>
#include <vector>
#include <cpp/data/Memory.h>

namespace cpp
{

    class String
		: public std::string
	{
	public:
        String()
            : std::string() { }
        String( size_type count, value_type ch, const allocator_type & alloc = allocator_type() )
            : std::string(count, ch, alloc) { }
        String( const value_type * s, const allocator_type & alloc = allocator_type() )
            : std::string(s, alloc) { }
        String( const value_type * s, size_type count, const allocator_type & alloc = allocator_type( ) )
            : std::string( s, count, alloc ) { }
        template< class InputIt > String( InputIt first, InputIt last, const allocator_type & alloc = allocator_type() )
            : std::string(first, last, alloc) { }
        String( const Memory & memory )
            : std::string( memory.begin(), memory.begin( ) + memory.length( ) ) { }
        String( const std::string & copy )
            : std::string(copy) { }
        String( const String & copy )
            : std::string(copy) { }
        String( std::string && move )
            : std::string(std::move(move)) { }
        String( String && move )
            : std::string(std::move(move)) { }

        static const String whitespace;
        typedef std::vector<String> Array;

        String & operator=( const Memory & memory )
            { std::string::assign( memory.begin(), memory.end() ); return *this; }
        String & operator=( const std::string & str )
            { std::string::operator=(str); return *this; }
        String & operator=( const String & str )
            { std::string::operator=(str); return *this; }
        String & operator=( std::string && str )
            { std::string::operator=( std::move( str) ); return *this; }
        String & operator=( String && str )
            { std::string::operator=( std::move( str ) ); return *this; }
        String & operator=( const value_type * s )
            { std::string::operator=(s); return *this; }
        String & operator=( value_type ch )
            { std::string::operator=(ch); return *this; }

        String & append( const char * begin )
            { std::string::append( begin ); return *this; }
        String & append( const char * begin, const char * end )
            { std::string::append( begin, end ); return *this; }
        String & append( const Memory & memory )
            { std::string::append( memory.begin(), memory.end() ); return *this; }
        String & append( char ch )
            { std::string::append( 1, ch ); return *this; }
        String & append( size_t count, char ch )
            { std::string::append( count, ch ); return *this; }

        String & operator+=( const char * begin )
            { std::string::append( begin ); return *this; }
        String & operator+=( const Memory & memory )
            { std::string::append( memory.begin(), memory.end() ); return *this; }
        String & operator+=( char ch )
            { std::string::append( 1, ch ); return *this; }

        bool isEmpty( ) const
            { return empty( ); }

        String substr( size_t offset, size_t count = npos ) const
            { return Memory{ *this }.substr( offset, count ); }
        String trim( const Memory & trimSet = whitespace ) const
            { return Memory{ *this }.trim( trimSet ); }
        String::Array split( const Memory &  delimSet, const Memory & trimSet = whitespace, bool ignoreEmpty = true ) const
            { auto result = Memory{ *this }.split( delimSet, trimSet, ignoreEmpty ); return String::Array{ result.begin(), result.end() }; }

        String & replaceAll( const Memory & pattern, const Memory & dst, size_t pos = 0 );
        String & replaceFirst( const Memory & pattern, const Memory & dst, size_t pos = 0 );
        String & replaceLast( const Memory & pattern, const Memory & dst, size_t pos = npos );

        String & toUpper( );
        String & toLower( );

        Memory::Match match( Memory regex ) const
            { return Memory{ *this }.match( regex ); }
        Memory::Match match( const std::regex & regex ) const
            { return Memory{ *this }.match(regex); }
        Memory::Match searchOne( Memory regex, bool isContinuous = false ) const
            { return Memory{ *this }.searchOne( regex ); }
        Memory::Match searchOne( const std::regex & regex, bool isContinuous = false ) const
            { return Memory{ *this }.searchOne( regex ); }
        Memory::Matches searchAll( Memory regex ) const
            { return Memory{ *this }.searchAll( regex ); }
        Memory::Matches searchAll( const std::regex & regex ) const
            { return Memory{ *this }.searchAll( regex ); }

        virtual String toString( ) const
            { return *this; }

        static String printf( const char * fmt, ... );

        template<typename T, typename... Params>
        static String format( const Memory & fmt, const T & param, Params... parameters );
        static String format( const Memory & fmt );

        static String pad( String s, int width, bool isLeftAligned = true, char ch = ' ' );
    };

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

    inline String toString( f64_t value )
    {
        char buffer[32];
        int len = snprintf( buffer, sizeof( buffer ), "%f", value );
        while ( len > 0 && buffer[len - 1] == '0' )
            { buffer[--len] = 0; }
        return String{ buffer };
    }

    inline String toString( f32_t value )
        { return toString( (f64_t)value ); }

    inline String toString( const char * value )
        { return String{ value }; }

    inline String toString( const std::string & value )
        { return String{ value }; }

    inline String toString( const Memory & value )
        { return String{ value }; }

    template<typename T, typename... Params>
	String String::format(const Memory & fmt, const T & param, Params... parameters)
	{
        String result;

        size_t begin = 0;
		size_t pos = 0;
		while (pos < fmt.length())
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

	inline String String::format(const Memory & fmt)
	{
		return fmt;
	}

}

inline bool operator==( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) == 0; }
inline bool operator==( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) == 0; }
inline bool operator==( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory{ lhs }.compare( rhs ) == 0; }
inline bool operator==( const cpp::Memory & lhs, const cpp::String & rhs )
    { return lhs.compare( cpp::Memory{ rhs } ) == 0; }
inline bool operator==( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) == 0; }

inline bool operator!=( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) != 0; }
inline bool operator!=( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) != 0; }
inline bool operator!=( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory{ lhs }.compare( rhs ) != 0; }
inline bool operator!=( const cpp::Memory & lhs, const cpp::String & rhs )
    { return lhs.compare( cpp::Memory{ rhs } ) != 0; }
inline bool operator!=( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) != 0; }

inline bool operator<( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) < 0; }
inline bool operator<( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) < 0; }
inline bool operator<( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory{ lhs }.compare( rhs ) < 0; }
inline bool operator<( const cpp::Memory & lhs, const cpp::String & rhs )
    { return lhs.compare( cpp::Memory{ rhs } ) < 0; }
inline bool operator<( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) < 0; }

inline bool operator<=( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) <= 0; }
inline bool operator<=( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) <= 0; }
inline bool operator<=( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory{ lhs }.compare( rhs ) <= 0; }
inline bool operator<=( const cpp::Memory & lhs, const cpp::String & rhs )
    { return lhs.compare( cpp::Memory{ rhs } ) <= 0; }
inline bool operator<=( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) <= 0; }

inline bool operator>( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) > 0; }
inline bool operator>( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) > 0; }
inline bool operator>( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory{ lhs }.compare( rhs ) > 0; }
inline bool operator>( const cpp::Memory & lhs, const cpp::String & rhs )
    { return lhs.compare( cpp::Memory{ rhs } ) > 0; }
inline bool operator>( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) > 0; }

inline bool operator>=( const cpp::String & lhs, const char * rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) >= 0; }
inline bool operator>=( const char * lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) >= 0; }
inline bool operator>=( const cpp::String & lhs, const cpp::Memory & rhs )
    { return cpp::Memory{ lhs }.compare( rhs ) >= 0; }
inline bool operator>=( const cpp::Memory & lhs, const cpp::String & rhs )
    { return lhs.compare( cpp::Memory{ rhs } ) >= 0; }
inline bool operator>=( const cpp::String & lhs, const cpp::String & rhs )
    { return cpp::Memory{ lhs }.compare( cpp::Memory{ rhs } ) >= 0; }

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

inline bool operator==( const cpp::String::Array & lhs, const cpp::Memory::Array & rhs )
    { return std::equal( lhs.begin( ), lhs.end( ), rhs.begin( ), []( const cpp::String & lhs, const cpp::Memory & rhs ) -> bool { return lhs == rhs; } ); }
inline bool operator==( const cpp::Memory::Array & lhs, const cpp::String::Array & rhs )
    { return std::equal( lhs.begin( ), lhs.end( ), rhs.begin( ), []( const cpp::Memory & lhs, const cpp::String & rhs ) -> bool { return lhs == rhs; } ); }

