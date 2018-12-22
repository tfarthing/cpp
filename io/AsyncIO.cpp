#ifndef TEST

#include <cpp/chrono/Timer.h>
#include <cpp/process/Thread.h>
#include <cpp/io/AsyncIO.h>

#define ASIO_STANDALONE
#define ASIO_HAS_STD_CHRONO
#include <lib/asio/asio.hpp>

namespace cpp
{

    struct AsyncIO::Detail
    {
        Detail( )
            : m_io( ), m_work( m_io ) { }

        ~Detail()
            { m_io.stop(); }

        asio::io_service m_io;
        asio::io_service::work m_work;
    };


    AsyncIO::AsyncIO( )
        : m_detail( std::make_shared<Detail>() )
    {
    }

    asio::io_service & AsyncIO::get( )
    {
        return m_detail->m_io;
    }

    /* no block, call all ready handlers */
    bool AsyncIO::poll( )
    {
        return m_detail->m_io.poll( ) != 0;
    }

    /* block until at lease one handler has been called */
    bool AsyncIO::runOne( )
    {
        if ( m_detail->m_io.run_one( ) )
            { m_detail->m_io.poll( ); return true; }
        return false;
    }

    /* block until at lease one handler has been called, or until the timeout expires */
    bool AsyncIO::runOne( Duration timeout )
    {
        if ( timeout.isInfinite() )
            { return runOne( ); }
        else if ( timeout.isNegative() )
            { return poll( ); }
        else
            { return runOne( Time::now( ) + timeout ); }
    }

    /* 
    This function will block until at lease one handler has been called, or until the timeout expires.
    
    Return: true if a handler was called before the timeout expired, otherwise false
    */
    bool AsyncIO::runOne( Time timeout )
    {
        if ( timeout < Time::now( ) )
            { return poll( ); }

        auto timer = asyncWait( timeout, nullptr );
        runOne( );
        return !timer.isExpired( );
    }

    bool AsyncIO::runFor( Duration duration )
    {
        return runUntil( Time::now( ) + duration );
    }

    bool AsyncIO::runUntil( Time timeout )
    {
        bool result = false;

        while ( Time::now() < timeout )
        { 
            if ( runOne( timeout ) )
                { result = true; }
        }

        return result;
    }

    void AsyncIO::stop( )
    {
        m_detail->m_io.stop( );
    }

    AsyncTimer AsyncIO::asyncWait( Time expireTime, std::function<void( const std::error_code & )> handler )
    {
        return AsyncTimer{ *this, expireTime, handler };
    }

    AsyncTimer AsyncIO::asyncWait( Duration duration, std::function<void( const std::error_code & )> handler )
    {
        return AsyncTimer{ *this, duration, handler };
    }



    struct AsyncTimer::Detail
    {
        Detail( asio::io_service & io, cpp::Time expireTime, std::function<void( const std::error_code & )> handler )
            : m_isPending( true ), m_isExpired(false), m_handler(std::move(handler)), m_timer( io )
        { 
            auto fromNow = std::max( cpp::Duration::zero, expireTime - cpp::Time::now( ));
            m_timer.expires_from_now( fromNow.std() );
            m_timer.async_wait( [&]( const std::error_code & error ) 
                { 
                    m_isPending = false;
                    std::function<void( const std::error_code & )> handler = std::move( m_handler );
                    assert( !m_handler );
                
                    m_isExpired = !error;
                    if ( handler )
                        { handler( error ); }
                } );
        }

        ~Detail()
        {  
            m_handler = nullptr;
            close( );
        }

        void close( )
        {
            if ( m_isPending )
            {
                std::error_code err; 
                m_timer.cancel( err );
                m_timer.get_io_service( ).poll( );
                while ( m_isPending )
                    { m_timer.get_io_service( ).run_one( );}
                assert( !m_isPending );
            }
        }

        asio::steady_timer m_timer;
        std::function<void( const std::error_code & )> m_handler;
        bool m_isPending;
        bool m_isExpired;
    };

    AsyncTimer::AsyncTimer( )
        : m_detail( nullptr )
    {
    }

    AsyncTimer::AsyncTimer( AsyncIO & parent, Time expireTime, std::function<void( const std::error_code & )> handler )
        : m_detail( std::make_shared<Detail>( parent.get( ), expireTime, handler ) )
    {
    }

    AsyncTimer::AsyncTimer( AsyncIO & parent, Duration duration, std::function<void( const std::error_code & )> handler )
        : m_detail( std::make_shared<Detail>( parent.get(), cpp::Time::inFuture( duration ), handler ) )
    {
    }

    bool AsyncTimer::isPending( ) const
    {
        return m_detail ? m_detail->m_isPending : false;
    }

    bool AsyncTimer::isExpired( ) const
    {
        return m_detail ? m_detail->m_isExpired : false;
    }

    //  User handler will not be triggered
    void AsyncTimer::cancel( )
    {
        if ( m_detail )
            { m_detail->close(); }
    }


}

#else

#include <cpp/meta/Unittest.h>
#include <cpp/io/AsyncIO.h>

SUITE( AsyncIO )
{
    using namespace cpp;

    TEST( AsyncTimer )
    {
        bool handled = false;
        AsyncIO io{};
        {
            //  (1) test that the handler gets called correctly
            AsyncTimer timer = io.asyncWait(
                cpp::Duration::ofMillis( 100 ),
                [&]( const std::error_code & error )
            {
                if ( !error )
                    { handled = true; }
            } );
            io.poll( );
            CHECK( timer.isExpired() == false );
            CHECK( handled == false );

            io.runFor( cpp::Duration::ofMillis( 110 ) );
            CHECK( timer.isExpired( ) == true );
            CHECK( handled == true );


            //  (2) test behavior with time in past
            handled = false;
            timer = io.asyncWait(
                cpp::Duration::ofMillis( -100 ),
                [&]( const std::error_code & error )
                    {
                        if ( !error )
                            { handled = true; }
                    } );
            io.runOne( );
            CHECK( timer.isExpired( ) == true );
            CHECK( handled == true );


            //  (3) test behavior with timer leaving scope before handler is triggered
            handled = false;
            timer = io.asyncWait(
                cpp::Duration::ofMillis( 100 ),
                [&]( const std::error_code & error )
            {
                if ( !error )
                    { handled = true; }
            } );
            io.poll( );
            CHECK( timer.isExpired( ) == false );
            CHECK( handled == false );
        }
        io.poll( );
        CHECK( handled == false );
    }
}

#endif
