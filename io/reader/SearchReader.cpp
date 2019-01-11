#include "../../data/DataBuffer.h"
#include "../Input.h"
#include "SearchReader.h"

namespace cpp
{

    void SearchReader::iterator::get( )
    {
        if ( m_reader )
        {
            m_cursor = m_reader->get( );
            if ( m_cursor.isNull( ) )
                { m_reader = nullptr; return; }
        }
    }


    struct SearchReader::Detail
    {
        Detail( const Memory & regex, const Input & input, size_t buflen, Duration timeout )
            : m_regex( regex.begin(), regex.end() ), m_input( input ), m_buffer( buflen ), m_timeout( timeout ) { }

        void tryRead( );
        bool trySearch( Cursor & result );

        Input m_input;
        std::regex m_regex;
        size_t m_position = 0;
        size_t m_lineNumber = 1;
        StringBuffer m_buffer;
        size_t m_findPos = 0;
        String m_fragment;
        bool m_resetFlag = false;
        Duration m_timeout;
    };

    SearchReader::SearchReader( const Memory & regex, const Input & input, size_t buflen, Duration timeout )
        : m_detail( std::make_shared<Detail>( regex, input, buflen, timeout ) ) { }

    bool SearchReader::isOpen( ) const
    {
        return m_detail->m_input.isOpen( ) || !m_detail->m_buffer.getable( ).isEmpty( );
    }

    SearchReader::iterator SearchReader::begin( )
    {
        return iterator( isOpen( ) ? this : nullptr );
    }

    SearchReader::iterator SearchReader::end( )
    {
        return iterator( nullptr );
    }

    SearchReader::Cursor SearchReader::get( )
    {
        Cursor result{ };
        while ( !m_detail->trySearch( result ) && isOpen( ) )
        {
            m_detail->tryRead( );
        }
        return result;
    }

    void SearchReader::Detail::tryRead( )
    {
        if ( m_input )
        {
            m_input.get( m_buffer );
        }
    }

    bool SearchReader::Detail::trySearch( Cursor & result )
    {
        Memory data = m_buffer.getable( );
        if ( data.length( ) )
        {
            /*
            if ( m_resetFlag )
            { m_fragment.clear(); m_resetFlag = false; }
            m_fragment += data;
            std::cmatch match;
            if ( std::regex_search( m_fragment.begin(), m_fragment.end(), match, m_regex ) )
            {
            match.
            }
            m_findPos = data.find_first_of( "\n", m_findPos );
            //  a match is successful in 2 cases:
            //  (1) '\n' is found in the input data
            //  (2) the input has been closed
            if ( m_findPos != Memory::npos || !m_input )
            {
            m_findPos = ( !m_input ) ? data.length( ) : m_findPos + 1;
            size_t fragmentLength = m_fragment.length();
            if ( fragmentLength == 0 )
            { result = Cursor{ m_position, m_lineNumber, m_buffer.get( m_findPos ).trim( "\r\n" ) }; }
            else
            { m_fragment += m_buffer.get( m_findPos ); m_resetFlag = true; result = Cursor{ m_position, m_lineNumber, Memory{ m_fragment }.trim( "\r\n" ) }; }
            m_position += fragmentLength + m_findPos;
            m_lineNumber++;
            m_findPos = 0;
            return true;
            }
            if ( data.length( ) == m_buffer.size( ) )
            {
            m_fragment += m_buffer.getAll();
            m_findPos = 0;
            }
            else
            {
            m_findPos = data.length();
            }
            */
        }
        return false;
    }

}

