#pragma once

#include <set>

#include "../../cpp/data/Memory.h"
#include "../../cpp/time/Duration.h"
#include "../../cpp/time/DateTime.h"
#include "../../cpp/process/Thread.h"
#include "../../cpp/process/Program.h"
#include "../../cpp/file/Files.h"
#include "../../cpp/file/FilePath.h"
#include "../../cpp/file/SyncFile.h"



namespace cpp
{

    enum class LogLevel
        { Alert, Error, Warning, Notice, Info, Debug, None };

	void log( std::string message );
	void log( LogLevel level, std::string message );
	template<typename... Params> void log( const Memory & fmt, Params... parameters );
	template<typename... Params> void log( LogLevel level, const Memory & fmt, Params... parameters );
	
	void debug( std::string message );
	template<typename... Params> void debug( const Memory & fmt, Params... parameters );

    class Logger
    {
    public:
        Logger( );
        ~Logger( );

		typedef std::function<void( DateTime time, LogLevel level, const std::string & message )> handler_t;

		void setConsole( LogLevel level );
		void setDebug( LogLevel level );
		void setFile( LogLevel level );
		void setFile( LogLevel level, std::string filename, std::function<void( FilePath )> onArchive );
		void setHandler( LogLevel level, handler_t handler );

        void log( LogLevel level, std::string message );
        void flush( );
        void close( );

    private:
        struct Entry
        {
            DateTime time;
            LogLevel level;
			std::string message;
        };
    private:
        void fn( );

		void openFile( );
        std::vector<Entry> dequeue( );
    private:
		LogLevel m_consoleLevel = LogLevel::None;
		LogLevel m_debugLevel = LogLevel::None;
		LogLevel m_fileLevel = LogLevel::None;
		LogLevel m_handlerLevel = LogLevel::None;
		handler_t m_handler;

		SyncFile m_file;
		std::string m_filename;
		FilePath m_filepath;
		DateTime m_archiveTime;
		std::function<void( FilePath )> m_archiveHandler;
        
		std::vector<Entry> m_queue;
        Thread m_thread;
		Mutex m_mutex;
	};



	std::string toString( LogLevel logLevel );
	const char * encodeLogLevel( LogLevel logLevel );

	LogLevel decodeLogLevel( const Memory & text );



    inline void log( std::string message )
    {
        log( LogLevel::Info, std::move( message ) );
    }


    inline void log( LogLevel level, std::string message )
    {
		cpp::Program::logger( ).log( level, std::move( message ) );
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


    inline void debug( std::string message )
    {
#ifdef _DEBUG
		log( LogLevel::Debug, std::move( message ) );
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

