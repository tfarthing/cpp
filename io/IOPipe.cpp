#include <cpp/Exception.h>
#include <cpp/Platform.h>
#include <cpp/chrono/Timer.h>
#include <cpp/process/Thread.h>
#include <cpp/process/Lock.h>
#include <cpp/io/IOPipe.h>
#include <cpp/io/StandardInput.h>
#include <cpp/platform/windows/WindowsException.h>
#include <cpp/util/DataBuffer.h>
#include <cpp/util/Log.h>

namespace cpp
{

    class PipeInput : public Input
    {
    public:
        PipeInput( )
            : Input( nullptr ) { }

        PipeInput( HANDLE pipe, size_t buflen )
            : Input( std::make_shared<Source>( pipe, buflen ) ) { }

        void start()
            { assert( m_source ); std::static_pointer_cast<Source>(m_source)->start(); }

        void detach()
            { assert( m_source ); std::static_pointer_cast<Source>(m_source)->detach(); }

        HANDLE handle()
            { return m_source ? std::static_pointer_cast<Source>(m_source)->handle() : nullptr; }

    private:
        class Source
            : public Input::Source_t
        {
        public:
            Source( HANDLE pipe, size_t buflen )
                : m_pipe( pipe ), m_buffer( buflen ) {  }

            void start()
            { 
                assert( !m_thread.isRunning( ) );
                m_thread = [=]( ) { fn( ); }; 
            }

            void detach()
            {
                m_thread.detach( );
            }

            bool isOpen( ) const override
                { auto lock = m_mutex.lock( ); return m_pipe != INVALID_HANDLE_VALUE || !m_buffer.getable().isEmpty(); }
            
            Memory read( const Memory & dst, Duration timeout ) override
            { 
                Timer timer;
                auto lock = m_mutex.lock( );
                while ( m_pipe != INVALID_HANDLE_VALUE || !m_buffer.getable().isEmpty() )
                { 
                    if ( m_buffer.getable( ).isEmpty( ) )
                    {
                        if ( timer.test( timeout ) )
                            { break; }
                        lock.waitFor( timer.until( timeout ) );
                        continue;
                    }
                    size_t len = std::min( dst.length(), m_buffer.getable().length() );
                    return Memory::copy( dst, m_buffer.get(len) );
                }
                return Memory::Null;
            }
            
            void close( ) override
            { 
                auto lock = m_mutex.lock( );
                if ( m_pipe != INVALID_HANDLE_VALUE )
                {
                    cpp::debug( "IOPipe::Source::close() - handle(%)", (uint64_t)m_pipe );
                    CloseHandle( m_pipe );
                    m_pipe = INVALID_HANDLE_VALUE;
                    m_buffer.getAll( ); // clear the recv buffer
                }
                m_mutex.notifyAll( );
            }

            HANDLE handle( )
                { return m_pipe; }
        private:
            void fn( )
            {
                cpp::Thread::setName( "IOPipe input thread" );
                while ( m_pipe != INVALID_HANDLE_VALUE )
                {
                    auto lock = m_mutex.lock( );

                    Memory dst = m_buffer.putable( );
                    if ( dst.isEmpty( ) )
                    {
                        lock.wait( );
                        continue;
                    }
                    
                    lock.unlock( );

                    DWORD bytes = 0;
                    BOOL result = ReadFile( m_pipe, (char *)dst.begin( ), (DWORD)dst.length( ), &bytes, NULL );
                    lock.lock( );
                    if ( !result )
                    {
                        cpp::debug( "IOPipe::Source::close() - handle(%)", (uint64_t)m_pipe );
                        CloseHandle( m_pipe );
                        m_pipe = INVALID_HANDLE_VALUE;
                    }
                    else
                    {
                        m_buffer.put( bytes );
                    }
 
                    lock.notifyAll( );
                }
            }
        private:
            HANDLE m_pipe;
            DataBuffer m_buffer;
            Thread m_thread;
            mutable Mutex m_mutex;
        };
    };

    class PipeOutput
        : public Output
    {
    public:
        PipeOutput()
            : Output( nullptr ) { }

        PipeOutput( HANDLE pipe, size_t buflen )
            : Output( std::make_shared<Sink>( pipe, buflen ) ) { }

        HANDLE handle()
            { return m_sink ? std::static_pointer_cast<Sink>(m_sink)->handle() : nullptr; }

    private:
        class Sink
            : public Output::Sink_t
        {
        public:
            Sink( HANDLE pipe, size_t buflen )
                : m_pipe( pipe ), m_buffer(buflen) { }
            bool isOpen( ) const override
                { return m_pipe != INVALID_HANDLE_VALUE; }
            Memory write( const Memory & src ) override
            {
                DWORD bytes = 0; 
                if ( isOpen( ) && WriteFile( m_pipe, src.begin( ), (DWORD)src.length( ), &bytes, NULL ) == FALSE )
                    { close(); }
                return isOpen( ) ? src.substr( 0, bytes ) : Memory::Null;
            }
            void flush( ) override
                {}
            void close( ) override
            {
                if ( isOpen( ) )
                {
                    cpp::debug( "IOPipe::Sink::close() - handle(%)", (uint64_t)m_pipe );
                    CloseHandle( m_pipe );
                    m_pipe = INVALID_HANDLE_VALUE;
                }
            }

            HANDLE handle( )
                { return m_pipe; }
        private:
            HANDLE m_pipe;
            DataBuffer m_buffer;
        };
    };

    struct IOPipe::Detail
    {
        Detail()
            : m_input( ), m_output( ) { }
        Detail( HANDLE input, HANDLE output, size_t buflen )
            : m_input(input, buflen), m_output(output, buflen) { }
        ~Detail()
            { m_output.close(); m_input.close(); }

        PipeInput m_input;
        PipeOutput m_output;
    };

    IOPipe::IOPipe( )
        : m_detail( std::make_shared<Detail>( ) )
    {
    }

    IOPipe::IOPipe( size_t buflen, bool startInputThread )
    {
        HANDLE input = NULL;
        HANDLE output = NULL;

        SECURITY_ATTRIBUTES attrs;
        attrs.nLength = sizeof( SECURITY_ATTRIBUTES );
        attrs.bInheritHandle = TRUE;
        attrs.lpSecurityDescriptor = NULL;

        windows::check( CreatePipe( &input, &output, &attrs, (DWORD)buflen ) != FALSE );

        m_detail = std::make_shared<Detail>( input, output, buflen );
        
        if ( startInputThread )
            { startInput(); }
    }

    IOPipe::IOPipe( std::shared_ptr<Detail> && detail )
        : m_detail( std::move( detail ) ) { }

    void IOPipe::startInput()
    {
        m_detail->m_input.start( );
    }

    Input & IOPipe::input( )
    {
        return m_detail->m_input;
    }
    Output & IOPipe::output( )
    {
        return m_detail->m_output;
    }

    void * IOPipe::getInputHandle( )
    {
        return m_detail->m_input.handle( );
    }
    
    void * IOPipe::getOutputHandle( )
    {
        return m_detail->m_output.handle( );
    }


    IOPipe IOPipe::createChildOutput( size_t buflen )
    {
        HANDLE input = NULL;
        HANDLE output = NULL;

        SECURITY_ATTRIBUTES attrs;
        attrs.nLength = sizeof( SECURITY_ATTRIBUTES );
        attrs.bInheritHandle = TRUE;
        attrs.lpSecurityDescriptor = NULL;

        windows::check( 
            CreatePipe( &input, &output, &attrs, (DWORD)buflen ) != FALSE &&
            SetHandleInformation( input, HANDLE_FLAG_INHERIT, 0 ) != FALSE );

        cpp::debug( "IOPipe - input(%), output(%)", (uint64_t)input, (uint64_t)output );

        return IOPipe{ std::make_shared<Detail>( input, output, buflen ) };
    }

    IOPipe IOPipe::createChildInput( size_t buflen )
    {
        HANDLE input = NULL;
        HANDLE output = NULL;

        SECURITY_ATTRIBUTES attrs;
        attrs.nLength = sizeof( SECURITY_ATTRIBUTES );
        attrs.bInheritHandle = TRUE;
        attrs.lpSecurityDescriptor = NULL;

        windows::check(
            CreatePipe( &input, &output, &attrs, (DWORD)buflen ) != FALSE &&
            SetHandleInformation( output, HANDLE_FLAG_INHERIT, 0 ) != FALSE );

        cpp::debug( "IOPipe - input(%), output(%)", (uint64_t)input, (uint64_t)output );

        return IOPipe{ std::make_shared<Detail>( input, output, buflen ) };
    }


    Input createStdin( size_t buflen )
    {
        Input input;
        HANDLE handle = GetStdHandle( STD_INPUT_HANDLE );
        if ( handle )
        {
            PipeInput pipe = PipeInput{ GetStdHandle( STD_INPUT_HANDLE ), buflen };
            pipe.start( );
            pipe.detach( );
            input = pipe;
        }
        return input;
    }

    Input & StandardInput::input( size_t buflen )
    {
        static Input instance = createStdin( buflen );
        return instance;
    }

}