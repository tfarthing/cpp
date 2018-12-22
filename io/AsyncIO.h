#pragma once

#include <memory>
#include <functional>

#include <cpp/Platform.h>
#include <cpp/chrono/Duration.h>
#include <cpp/chrono/Time.h>

namespace asio
{
    class io_service;
}

namespace cpp
{

    /*
    AsyncTimer
        * Will call the handler on first call to AsyncIO::poll() after the timer has expired.
        * Timeout values in the past will expire immediately on first call to AsyncIO.poll()
        * Calling cancel() before the handler has been called will gaurantee that the handler 
            will not be called.
    */
    class AsyncTimer
    {
    public:
        AsyncTimer( );

        bool isPending( ) const;
        bool isExpired( ) const;
        void cancel( );

    private:
        struct Detail;
        std::shared_ptr<Detail> m_detail;

        friend class AsyncIO;
        AsyncTimer( AsyncIO & parent, Time expireTime, std::function<void( const std::error_code & )> handler );
        AsyncTimer( AsyncIO & parent, Duration duration, std::function<void( const std::error_code & )> handler );
    };

    class AsyncIO
    {
    public:
        AsyncIO( );

        asio::io_service & get( );

        bool poll( );

        bool runOne( );
        bool runOne( Duration timeout );
        bool runOne( Time timeout );

        bool runFor( Duration duration );
        bool runUntil( Time time );

        AsyncTimer asyncWait( Time expireTime, std::function<void( const std::error_code & )> handler );
        AsyncTimer asyncWait( Duration duration, std::function<void( const std::error_code & )> handler );

        void stop( );

    private:
        struct Detail;
        std::shared_ptr<Detail> m_detail;
    };

}