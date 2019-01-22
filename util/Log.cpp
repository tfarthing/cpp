#ifndef TEST

#include "../process/Program.h"
//#include <cpp/io/StandardOutput.h>
#include "Log.h"
#include "LogFilter.h"


namespace cpp
{

    const char * toString( LogLevel level )
    {
        switch (level)
        {
        case LogLevel::Emerg:         return "emerg";
        case LogLevel::Alert:         return "alert";
        case LogLevel::Crit:          return "crit";
        case LogLevel::Error:         return "error";
        case LogLevel::Warning:       return "warning";
        case LogLevel::Notice:        return "notice";
        case LogLevel::Info:          return "info";
        case LogLevel::Debug:         return "debug";
        default:                      return "null";
        }
    }

    LogLevel parseLogLevel(const Memory & str)
    {
        const LogLevel levels[] = 
            { LogLevel::Null, LogLevel::Emerg, LogLevel::Alert, LogLevel::Crit, LogLevel::Error, LogLevel::Warning, LogLevel::Notice, LogLevel::Info, LogLevel::Debug };
        for (LogLevel level : levels)
        {
            if (str == toString(level))
                { return level; }
        }
        return LogLevel::Null;
    }

    Logger & logger()
        { return Program::logger(); }

    Logger::Logger()
        : m_running(true), m_thread( [=](){ fn(); } ) { }

    Logger::~Logger()
    { 
        try
        {
            close( );
        }
        catch ( cpp::IOException & )
        {
            //
        }
    }

    void Logger::flush( )
    {
        auto lock = m_mutex.lock( );
        while ( !m_queue.empty( ) )
            { lock.waitFor( cpp::Duration::ofMillis(50) ); }
        
        for ( auto & filter : m_filters )
            { filter->flush(); }
    }

    void Logger::close()
    {
        auto lock = m_mutex.lock( );
        m_running = false;
        lock.notifyAll( );
        lock.unlock( );

        m_thread.join();

        filter(0);                          //  finish processing items in the queue
        m_filters.clear( );
    }

    void Logger::log(LogLevel level, String category, String message)
    {
        auto lock = m_mutex.lock();
        m_queue.emplace_back( DateTime::now(), level, std::move(category), std::move(message));
        lock.notifyAll();
    }

    void Logger::addFilter( LogFilter::ptr_t filter )
    {
        m_filters.insert( std::move( filter ) );
    }

    void Logger::removeFilter( const LogFilter::ptr_t & filter )
    {
        m_filters.erase( filter );
    }

    void Logger::addDebug( LogLevel level )
    {
        logger( ).addFilter( DebugLogFilter{ level } );
    }

    void Logger::addStdout( LogLevel level )
    {
        logger( ).addFilter( OutputLogFilter{ StandardOutput::output(), level } );
    }

    void Logger::addFile( FilePath dir, String name, String ext, LogLevel level )
    {
        logger( ).addFilter( FileLogFilter{ dir, name, ext, level } );
    }


    void Logger::fn( )
    {
        Thread::setName( "Logger" );
        while ( m_running )
        {
            filter( Duration::ofSeconds( 1 ) );
        }
    }

    void Logger::filter(Duration maxWait)
    {
        std::vector<Entry> queue = dequeue(maxWait);
        for (Entry & entry : queue)
        {
            for (auto & filter : m_filters)
            {
                filter->log( entry.m_time, entry.m_level, entry.m_category, entry.m_message );
            }
        }
    }

    std::vector<Logger::Entry> Logger::dequeue(Duration maxWait)
    {
        auto lock = m_mutex.lock();
        while ( m_running && m_queue.empty( ) )
            { lock.wait( ); }

        std::vector<Entry> queue;
        std::swap( m_queue, queue );
        return queue;
    }

}

#else

#include <cpp/util/Log.h>
#include <cpp/util/LogFilter.h>
#include <cpp/meta/Unittest.h>

SUITE( Logger )
{
    using namespace cpp;

    struct TestFilter_t : public cpp::AbstractLogFilter
    {
        TestFilter_t()
            : cpp::AbstractLogFilter( LogLevel::Info ) 
        {
        }

        void log( DateTime time, LogLevel level, const String & category, const String & message ) override
        {
            Memory::Array lines = split( message );
            for ( auto line : lines )
            {
                m_lines.push_back( format( time, level, category, line) );
            }
        }

        String::Array m_lines;
    };
    typedef Handle<TestFilter_t> TestFilter;

    TEST( logger )
    {
        TestFilter filter;

        {
            Logger logger;
            logger.addFilter( filter );

            const char * samples[3] =
            {
                "this is a short sample",
                "this is a long sample... blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah",
                "this is a multi-line string...\nline 1\nline2\n"
            };

            int count = 5;
            for ( int i = 0; i<count; i++ )
            {
                for ( auto sample : samples )
                {
                    logger.log( LogLevel::Info, "stdout", sample );
                }
            }
            logger.close( );

            CHECK( filter->m_lines.size( ) == 5 * count );
        }
    }
}

#endif
