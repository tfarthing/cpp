#ifndef TEST

#include "../../cpp/io/LineReader.h"
#include "../../cpp/io/Input.h"


namespace cpp
{

    bool LineReader::iterator::tryRead( )
    {
        if ( !m_input || !m_input->isOpen( ) )
            { return false; }

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
        Memory line = ( !m_input || !m_input->isOpen( ) )
            ? m_buffer.getAll( )
            : m_buffer.getLine( "\n", m_findPos );

        if ( line.isNull() )
        { 
            m_findPos = m_buffer.getable( ).length( ); 
            return false; 
        }

        m_cursor.position = m_nextPosition;
        m_cursor.lineNumber = m_nextLine;
        m_cursor.line = line;

        m_nextPosition += line.length( );
        m_nextLine++;
        m_findPos = 0;

        return true;
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
