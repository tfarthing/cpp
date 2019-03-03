#ifndef TEST

#include "../../cpp/io/LineReader.h"
#include "../../cpp/io/Input.h"


namespace cpp
{

    bool LineReader::iterator::tryRead( )
    {
        if ( !m_input->isOpen( ) )
        {
            return false;
        }

        //  if data is read return true;
        Memory bytes = m_input->readsome( m_buffer.putable( ) );
        if ( bytes )
        {
            m_buffer.put( bytes.length( ) );
            return true;
        }

        m_input->close( );
        m_input = nullptr;;
        return false;
    }


    bool LineReader::iterator::tryFind( )
    {
        Memory data = m_buffer.getable( );
        if ( data )
        {
            m_findPos = data.find_first_of( "\n", m_findPos );
            //  a match is successful in 2 cases:
            //  (1) '\n' is found in the input data
            //  (2) the input has been closed
            if ( m_findPos != Memory::npos || !m_input || !m_input->isOpen( ) )
            {
                // include the '\n' char in the line result
                m_findPos = ( m_findPos < data.length( ) )
                    ? m_findPos + 1
                    : data.length( );

                m_cursor.position = m_nextPosition;
                m_cursor.lineNumber = m_nextLineNumber;
                m_cursor.line = m_buffer.get( m_findPos );

                m_nextPosition += m_findPos;
                m_nextLineNumber++;
                m_findPos = 0;

                return true;
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
