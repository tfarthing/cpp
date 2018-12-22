#ifndef TEST

#include <cassert>

#include "DataBuffer.h"

namespace cpp
{

    Memory ReadBuffer::getAll( )
    {
        Memory result = getable( );
        m_getIndex += result.length( );
        return result;
    }

    Memory ReadBuffer::get( size_t bytes )
    {
        checkRead( m_getIndex, bytes );
        Memory result = getable( ).substr( 0, bytes );
        m_getIndex += result.length( );
        return result;
    }

    Memory ReadBuffer::getAt( size_t pos, size_t bytes )
    {
        checkRead( pos, bytes );
        Memory result = m_getBuffer.substr( pos, bytes );
        m_getIndex = pos + bytes;
        return result;
    }

    //  Reads a line (up to '\n'), or null if non-found
    Memory ReadBuffer::getLine( Memory delim, size_t pos )
    {
        pos = getable( ).find( delim, pos );
        return ( pos != Memory::npos ) ? get( pos + 1 ) : Memory::Null;
    }

    RegexMatch<Memory> ReadBuffer::getRegex( Memory regex )
    {
        return getRegex( std::regex{ regex.begin(), regex.end() } );
    }

    RegexMatch<Memory> ReadBuffer::getRegex( const std::regex & regex )
    {
        RegexMatch<Memory> result = getable( ).searchOne( regex, true );
        if ( result.hasMatch( ) )
        {
            assert( result.at( 0 ).begin( ) == getable( ).begin( ) );
            get( result.at( 0 ).length( ) );
        }
        return result;
    }



    DataBuffer::DataBuffer( Memory data )
        : ReadBuffer( m_buffer, false ), m_buffer( data )
    {
        m_getBuffer = m_buffer;
        m_putIndex = size( );
    }

    DataBuffer::DataBuffer( size_t bufsize )
        : ReadBuffer( m_buffer, false )
    {
        resize( bufsize );
    }

    Memory ReadBuffer::putable( )
    {
        if ( m_putIndex == m_getBuffer.length( ) && m_getIndex != 0 )
            { trim( ); }
        return Memory{ m_getBuffer.c_str( ) + m_putIndex, m_getBuffer.length( ) - m_putIndex };
    }

    void ReadBuffer::trim( )
    {
        if ( m_getIndex != 0 )
        {
            size_t len = m_putIndex - m_getIndex;
            if ( len != 0 )
                { memmove( (char *)m_getBuffer.c_str( ), m_getBuffer.c_str( ) + m_getIndex, len ); }
            m_putIndex = len;
            m_getIndex = 0;
        }
    }

    Memory ReadBuffer::put( size_t len )
    {
        checkWrite( m_putIndex, len );
        Memory result = Memory{ m_getBuffer.c_str( ) + m_putIndex, len };
        m_putIndex += len;
        return result;
    }

    Memory ReadBuffer::put( const Memory & memory )
    { 
        Memory dst = putable( ); 
        checkWrite( m_putIndex, memory.length( ) ); 
        Memory result = Memory::copy( dst, memory ); 
        m_putIndex += memory.length( ); 
        return result; 
    }


}


#else

#include <cpp/meta/Unittest.h>
#include <cpp/util/DataBuffer.h>

SUITE( DataBuffer )
{
    using namespace cpp;

    TEST( encode_primitives )
    {
        DataBuffer buffer(64);

        int32_t encode1 = -2300077;
        buffer.putBinary( encode1 );
        int32_t decode1;
        buffer.getBinary( decode1 );

        CHECK( decode1 == encode1 );

        f64_t encode2 = 3.14167 / 377;
        buffer.putBinary( encode2 );
        f64_t decode2;
        buffer.getBinary( decode2 );

        CHECK( decode2 == encode2 );
    }

    TEST( encode_string )
    {
        DataBuffer buffer(64);

        cpp::String encode1 = "Hello World!";
        buffer.putBinary( encode1 );
        cpp::String decode1;
        buffer.getBinary( decode1 );

        CHECK( decode1 == encode1 );
    }

}

#endif