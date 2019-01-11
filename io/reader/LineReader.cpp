#ifndef TEST

#include "../../data/DataBuffer.h"
#include "../Input.h"
#include "LineReader.h"

namespace cpp
{

    void LineReader::iterator::get( )
    {
        if ( m_reader )
        {
            m_cursor = m_reader->get( );
            if ( m_cursor.isNull() && m_reader->isOpen( ) == false )
                { m_reader = nullptr; return; }
        }
    }


    struct LineReader::Detail
    {
        Detail( const Input & input, size_t buflen, Duration timeout )
            : m_input( input ), m_buffer( buflen ), m_timeout( timeout ) { }

        bool tryRead( );
        bool tryFind( Cursor & result, bool flush = false );

        Input m_input;
        size_t m_position = 0;
        size_t m_lineNumber = 1;
		StringBuffer m_buffer;
        size_t m_findPos = 0;
        String m_fragment;
        bool m_resetFlag = false;
        Duration m_timeout;
    };

    LineReader::LineReader( const Input & input, size_t buflen, Duration timeout )
        : m_detail( std::make_shared<Detail>( input, buflen, timeout ) )
    {
        m_detail->m_input = input;
        m_detail->m_buffer.resize( buflen );
    }

    bool LineReader::isOpen( ) const
    {
        return m_detail->m_input.isOpen( ) || !m_detail->m_buffer.getable( ).isEmpty( );
    }

    LineReader::iterator LineReader::begin( )
    {
        return iterator( isOpen( ) ? this : nullptr );
    }

    LineReader::iterator LineReader::end( )
    {
        return iterator( nullptr );
    }

    String::Array LineReader::getAll( )
    {
        String::Array result; 
        for ( auto cursor : *this ) 
            { result.push_back( cursor.line( ) ); } 
        return result;
    }

    LineReader::Cursor LineReader::flush( )
    {
        Cursor result{ };
        m_detail->tryFind( result, true );
        return result;
    }

    LineReader::Cursor LineReader::get( )
    {
        Cursor result{ };
        while ( !m_detail->tryFind( result ) && m_detail->tryRead( ) );
        return result;
    }

    bool LineReader::Detail::tryRead( )
    {
        if (!m_input)
            { return false; }
        //  if data is read return true;
        if ( m_input.read( m_buffer, m_timeout ) )
            { return true; }
        //  if input closed due to the read return true
        return !m_input.isOpen(); 
    }

    bool LineReader::Detail::tryFind( Cursor & result, bool flushFlag )
    {
        Memory data = m_buffer.getable( );
        if ( data.length( ) )
        {
            if ( m_resetFlag )
            {
                m_fragment.clear( ); m_resetFlag = false;
            }
            m_findPos = data.find_first_of( "\n", m_findPos );
            //  a match is successful in 2 cases:
            //  (1) '\n' is found in the input data
            //  (2) the input has been closed
            if ( m_findPos != Memory::npos || !m_input || flushFlag )
            {
                m_findPos = ( m_findPos == Memory::npos || m_findPos == data.length() ) ? data.length( ) : m_findPos + 1;
                size_t fragmentLength = m_fragment.length( );
                if ( fragmentLength == 0 )
                {
                    result = Cursor{ m_position, m_lineNumber, m_buffer.get( m_findPos ).trim( "\r\n" ) };
                }
                else
                {
                    m_fragment += m_buffer.get( m_findPos ); 
                    m_resetFlag = true; 
                    result = Cursor{ m_position, m_lineNumber, Memory{ m_fragment }.trim( "\r\n" ) };
                }
                m_position += fragmentLength + m_findPos;
                m_lineNumber++;
                m_findPos = 0;
                return true;
            }
            if ( data.length( ) == m_buffer.size( ) )
            {
                m_fragment += m_buffer.getAll( );
                m_findPos = 0;
            }
            else
            {
                m_findPos = data.length( );
            }
        }
        return false;
    }

}

#else

#include <cpp/process/Thread.h>
#include <cpp/io/IOBuffer.h>
#include <cpp/io/reader/LineReader.h>
#include <cpp/meta/Unittest.h>
#include <cpp/util/log.h>

SUITE( LineReader )
{
    using namespace cpp;

    TEST( get )
    {
        IOBuffer io;

        Thread thread
        {
            [&io]() 
            {
                int lineCount = 0;
                auto input = io.input( );
                auto reader = LineReader{ input, 8 };
                for ( auto cursor : reader )
                {
                    if ( cursor.lineNumber() == 1 )
                        { CHECK( cursor.line() == "line #1" && cursor.position() == 0 ); }
                    else if ( cursor.lineNumber() == 2 )
                        { CHECK( cursor.line() == "long line #2" && cursor.position( ) == 8 ); }
                    else if ( cursor.lineNumber( ) == 3 )
                        { CHECK( cursor.line() == "line 3" && cursor.position( ) == 21 ); }
                    else if ( cursor.lineNumber( ) == 4 )
                        { CHECK( cursor.line() == "line #4" && cursor.position( ) == 28 ); }
                    else
                        { continue; }
                    lineCount++;
                }
                CHECK( lineCount == 4 );
            }
        };

        auto output = io.output( );
        output.write(
            "line #1\n"
            "long line #2\n"
            "line 3\n"
            "line #4");
        output.flush( );
        output.close( );

        thread.join( );
    }
}

#endif
