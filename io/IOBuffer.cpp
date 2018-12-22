#ifndef TEST

#include <cpp/chrono/Timer.h>
#include <cpp/io/IOBuffer.h>

namespace cpp
{

    bool IOBuffer::BufferedInput::Source::isOpen( ) const
    {
        auto lock = m_io.m_mutex.lock( );
        return m_io.m_isOpen || !m_io.m_buffer.getable( ).isEmpty( );
    }

    Memory IOBuffer::BufferedInput::Source::read( const Memory & dst, Duration timeout )
    {
        Timer timer;
        auto & buffer = m_io.m_buffer;
        auto lock = m_io.m_mutex.lock( );
        while ( m_io.m_isOpen )
        {
            //  wait if the read should block
            if ( buffer.getable( ).isEmpty( ) )
            { 
                if ( timer.test( timeout ) )
                    { break; }
                lock.waitFor( timer.until( timeout ) );
                continue; 
            }

            //  check if write will be unblocked
            bool notifyFlag = buffer.putable( ).isEmpty( );

            //  copy data
            size_t len = std::min( dst.length( ), buffer.getable( ).length( ) );
            Memory result = Memory::copy( dst, buffer.get( len ) );

            //  notify if write or flush becomes unblocked
            if ( notifyFlag || buffer.getable( ).isEmpty( ) )
                { lock.notifyAll( ); }

            return result;
        }
        return Memory::Null;
    }

    void IOBuffer::BufferedInput::Source::close( )
    {
        auto lock = m_io.m_mutex.lock( );
        m_io.m_isOpen = false;
        lock.unlock( );
        lock.notifyAll( );
    }


    bool IOBuffer::BufferedOutput::Sink::isOpen( ) const
    {
        auto lock = m_io.m_mutex.lock( );
        return m_io.m_isOpen;
    }

    Memory IOBuffer::BufferedOutput::Sink::write( const Memory & src )
    {
        auto & buffer = m_io.m_buffer;
        auto lock = m_io.m_mutex.lock( );
        while ( m_io.m_isOpen )
        {
            if ( buffer.putable( ).isEmpty( ) )
            {
                lock.wait( );
                continue;
            }

            //  check if read will be unblocked
            bool notifyFlag = buffer.getable( ).isEmpty( );

            //  copy data
            size_t len = std::min( src.length( ), buffer.putable( ).length( ) );
            Memory result = buffer.put( src.substr( 0, len ) );

            //  notify if read become unblocked
            if ( notifyFlag )
            {
                lock.notifyAll( );
            }
            return result;
        }
        return Memory::Null;
    }

    void IOBuffer::BufferedOutput::Sink::flush( )
    {
        auto lock = m_io.m_mutex.lock( );
        while ( m_io.m_isOpen && !m_io.m_buffer.getable( ).isEmpty( ) )
        {
            lock.wait( );
        }
    }

    void IOBuffer::BufferedOutput::Sink::close( )
    {
        auto lock = m_io.m_mutex.lock( );
        m_io.m_isOpen = false;
        lock.unlock( );
        lock.notifyAll( );
    }

}

#else

#include <cpp/process/Thread.h>
#include <cpp/io/IOBuffer.h>
#include <cpp/meta/Unittest.h>

SUITE( IOBuffer )
{
    using namespace cpp;

    TEST( read_write )
    {
        IOBuffer io( 8 );

        Thread thread
        {
            [&io]( )
            {
                auto input = io.input();
                String buffer( 8, '\0' );
                int readIndex = 0;
                int totalReads = 2;
                while ( input.isOpen() )
                {
                    Memory bytes = input.read(buffer);
                    if ( bytes.isNull( ) )
                        { break; }
                    if (readIndex == 0)
                        { CHECK( bytes == "line 1\n" ); }
                    else if (readIndex == 1)
                        { CHECK( bytes == "l" || bytes == "line 2\n" ); }
                    else if (readIndex == 2)
                        { CHECK( bytes == "ine 2\n" ); totalReads = 3; }
                    else
                        { CHECK( false ); }
                    readIndex++;
                }
                CHECK(readIndex == totalReads);
            }
        };

        auto output = io.output( );
        
        CHECK( output.write( "line 1\n" ).length() == 7 );
        output.flush();

        CHECK( output.write( "line 2\n" ).length( ) == 1 );
        CHECK( output.write( "ine 2\n" ).length( ) == 6 );
        output.flush( );

        output.close();
        thread.join( );
    }
}


#endif
