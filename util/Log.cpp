#ifndef TEST

#include "../process/Program.h"
#include "../../cpp/process/Platform.h"
#include "../../cpp/text/Utf16.h"
#include "Log.h"


namespace cpp
{

	std::string toString( LogLevel logLevel )
	{
		return encodeLogLevel( logLevel );
	}

    const char * encodeLogLevel( LogLevel level )
    {
        switch (level)
        {
			case LogLevel::Alert:			return "alert";
			case LogLevel::Error:			return "error";
			case LogLevel::Warning:			return "warning";
			case LogLevel::Notice:			return "notice";
			case LogLevel::Info:			return "info";
			case LogLevel::Debug:			return "debug";
			case LogLevel::None:			return "none";
			default:						
				assert( false );
				return "unknown";
        }
    }


    LogLevel decodeLogLevel(const Memory & str)
    {
        const LogLevel levels[] = 
            { LogLevel::Alert, LogLevel::Error, LogLevel::Warning, LogLevel::Notice, LogLevel::Info, LogLevel::Debug, LogLevel::None };

        for (LogLevel level : levels)
        {
            if (str == toString(level))
                { return level; }
        }

        return LogLevel::None;
    }

    

	Logger::Logger()
        : m_thread( ) 
	{
	}


    Logger::~Logger()
    { 
        try
			{ close( ); }
        catch ( cpp::IOException & )
			{ }
    }


	void Logger::setConsole( LogLevel level )
	{
		m_consoleLevel = level;
	}


	void Logger::setDebug( LogLevel level )
	{
		m_debugLevel = level;
	}


	void Logger::setFile( LogLevel level )
	{
		m_fileLevel = level;
	}

	void Logger::openFile( )
	{
		m_archiveTime = DateTime::trimAtDay( DateTime::now( ) ) + Duration::ofDays( 1 );
		m_filepath = cpp::format( m_filename, DateTime::now( ).toString( "%Y-%m-%d" ) );
		m_file.append( m_filepath );
	}

	void Logger::setFile( LogLevel level, std::string filename, std::function<void( FilePath )> onArchive )
	{
		m_fileLevel = level;
		m_filename = std::move( filename );
		m_archiveHandler = std::move( onArchive );

		openFile( );
	}


	void Logger::setHandler( LogLevel level, handler_t handler )
	{
		m_handlerLevel = level;
		m_handler = handler;
	}


	void Logger::log( LogLevel level, std::string message )
	{
		auto lock = m_mutex.lock( );
        if ( !m_thread.isRunning() )
            { m_thread = [=]( ) { fn( ); }; }
		m_queue.emplace_back( Entry{ DateTime::now( ), level, std::move( message ) } );
		lock.notifyAll( );
	}


	void Logger::flush( )
	{
		auto lock = m_mutex.lock( );
		while ( !m_queue.empty( ) )
		{
			lock.waitFor( cpp::Duration::ofMillis( 50 ) );
		}
		m_file.flush( );
	}


	void Logger::close( )
	{
		m_thread.interrupt( );
		m_thread.join( );
	}


    void Logger::fn( )
    {
        Thread::setName( "Logger" );
        while ( true )
        {
			std::vector<Entry> queue = dequeue( );
			for ( Entry & entry : queue )
			{
				String line = cpp::format( "% % %\r\n",
					entry.time.toString( ),
					encodeLogLevel( entry.level ),
					entry.message );

				if ( m_fileLevel <= entry.level )
				{
					if ( entry.time > m_archiveTime )
					{
						m_file.close( );
						m_archiveHandler( m_filepath );
						openFile( );
					}
					m_file.write( line );
				}
				
				if ( m_consoleLevel <= entry.level )
					{ puts( line.begin( ) ); }

				if ( m_debugLevel <= entry.level )
					{ OutputDebugString( toUtf16( line ).c_str( ) ); }

				if ( m_handlerLevel <= entry.level )
					{ m_handler( entry.time, entry.level, entry.message ); }
			}
        }
    }

    std::vector<Logger::Entry> Logger::dequeue( )
    {
        auto lock = m_mutex.lock();
        while ( m_queue.empty( ) )
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
