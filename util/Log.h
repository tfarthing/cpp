#pragma once

#include <set>
#include <cpp/String.h>
#include <cpp/chrono/Duration.h>
#include <cpp/chrono/DateTime.h>
#include <cpp/process/Thread.h>
#include <cpp/io/file/FilePath.h>

namespace cpp
{

    enum class LogLevel
        { Null, Emerg, Alert, Crit, Error, Warning, Notice, Info, Debug };

    const char * toString( LogLevel logLevel );
    LogLevel parseLogLevel( const Memory & text );

    struct LogFilter
    { 
        using ptr_t = std::shared_ptr<LogFilter>;
        virtual ~LogFilter( ) { }
        virtual void log( DateTime time, LogLevel level, const String & category, const String & message ) = 0; 
        virtual void flush( ) { } 
    };

    class Logger
    {
    public:
        Logger( );
        ~Logger( );

        void log( LogLevel level, String category, String message );
        void flush( );
        void close( );

        void addFilter( LogFilter::ptr_t filter );
        void removeFilter( const LogFilter::ptr_t & filter );

        static void addDebug( LogLevel level = LogLevel::Debug );
        static void addStdout( LogLevel level = LogLevel::Info );
        static void addFile( FilePath dir, String name = "", String ext = "log.txt", LogLevel level = LogLevel::Info );

    private:
        struct Entry
        {
            Entry( DateTime time, LogLevel level, String category, String message )
                : m_time( time ), m_level( level ), m_category( std::move( category ) ), m_message( message ) { }
            
            DateTime m_time;
            LogLevel m_level;
            String m_category;
            String m_message;
        };
    private:
        void fn( );

        void filter( Duration timeout );
        std::vector<Entry> dequeue(Duration timeout);
    private:
        std::atomic_bool m_running;
        std::set<LogFilter::ptr_t> m_filters;
        std::vector<Entry> m_queue;
        Mutex m_mutex;
        Thread m_thread;
    };

    Logger & logger();

    inline void xlog( LogLevel level, String category, String message )
    {
        logger( ).log( level, std::move( category ), std::move( message ) );
    }

    inline void log( String message )
    {
        xlog( LogLevel::Info, "", std::move( message ) );
    }

    inline void log( LogLevel level, String message )
    {
        xlog( level, "", std::move( message ) );
    }

    template<typename... Params> 
    void log( const Memory & fmt, Params... parameters )
    {
        log( String::format( fmt, parameters... ) );
    }

    template<typename... Params> 
    void log( LogLevel level, const Memory & fmt, Params... parameters )
    {
        log( level, String::format( fmt, parameters... ) );
    }

    inline void xdebug( String category, String message )
    {
#ifdef _DEBUG
        xlog( LogLevel::Debug, std::move( category ), std::move( message ) );
#endif
    }

    inline void debug( String message )
    {
#ifdef _DEBUG
        xlog( LogLevel::Debug, "", std::move( message ) );
#endif
    }

    template<typename... Params> 
    void debug( const Memory & fmt, Params... parameters )
    {
#ifdef _DEBUG
        debug( String::format( fmt, parameters... ) );
#endif
    }

}

