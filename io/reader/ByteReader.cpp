#include "../Input.h"
#include "ByteReader.h"

namespace cpp
{


    void ByteReader::iterator::get( )
    {
        if ( m_reader )
        {
            if ( !m_reader->isOpen( ) )
                { m_reader = nullptr; return; }
            m_cursor = m_reader->get( );
        }
    }


    struct ByteReader::Detail
    {
        Detail( const Input & input, size_t buflen, Duration timeout )
            : m_input( input ), m_buffer( buflen, '\0' ), m_timeout(timeout) { }

        void tryRead( );
        bool tryFind( Cursor & result );
        
        Input m_input;
        size_t m_position = 0;
        String m_buffer;
        Duration m_timeout;
    };

	ByteReader::ByteReader( const Input & input, size_t buflen, Duration timeout )
		: m_detail( std::make_shared<Detail>( input, buflen, timeout ) )
    { 
    }

    bool ByteReader::isOpen( ) const
    {
        return m_detail->m_input.isOpen( );
    }

    ByteReader::iterator ByteReader::begin( )
    {
        return iterator( isOpen( ) ? this : nullptr );
    }

    ByteReader::iterator ByteReader::end( )
    {
        return iterator( nullptr );
    }

    ByteReader::Cursor ByteReader::get( )
    {
        if ( m_detail->m_input )
        {
            Cursor result{ m_detail->m_position, m_detail->m_input.read( m_detail->m_buffer, m_detail->m_timeout ) };
            m_detail->m_position += result.data( ).length( );
            return result;
        }
        return Cursor{ m_detail->m_position, nullptr };
    }

    String ByteReader::getAll( )
    {
        String result; for ( auto cursor : *this ) 
            { result += cursor.data( ); } 
        return result;
    }

}

