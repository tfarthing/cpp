#pragma once

#include "../../cpp/time/DateTime.h"
#include "../../cpp/file/SyncFile.h"
#include "../../cpp/util/Log.h"



namespace cpp
{

    class AbstractLogFilter
        : public LogFilter
    {
        static const uint32_t CategoryAvgCount = 100;
    public:
        AbstractLogFilter( LogLevel level )
            : m_level( level ), m_categoryAvgSum( 16 * CategoryAvgCount ), m_headerFlag( true ) { }

        AbstractLogFilter & withLevel( LogLevel level )
            { m_level = level; return *this; }
        AbstractLogFilter & withHeader( bool headerFlag )
            { m_headerFlag = headerFlag; return *this; }

        String format( DateTime time, LogLevel level, const Memory & category, const Memory & message );
        Memory::Array split( const Memory & message );

    private:
        uint32_t categoryAvgLength( ) const
            { return m_categoryAvgSum / CategoryAvgCount; }
        uint32_t adjustCategoryAvgSum( uint32_t length )
            { m_categoryAvgSum -= categoryAvgLength( ); m_categoryAvgSum += length; return categoryAvgLength( ); }

    protected:
        LogLevel m_level;
        DateTime m_lastTime;
        cpp::String m_lastTimeString;
        uint32_t m_categoryAvgSum;
        LogLevel m_appendLevel;
        String m_appendCategory;
        bool m_headerFlag;
    };


    //  12-11-03 13:21:01.034:abcd level category message
    class OutputLogFilter_t
        : public AbstractLogFilter
    {
    public:
        OutputLogFilter_t( Output output, LogLevel level = LogLevel::Info )
            : AbstractLogFilter( level ), m_output( std::move( output ) ) { }

        void setOutput( Output output )
            { m_output = std::move( output ); }

        void log( DateTime time, LogLevel level, const String & category, const String & message ) override;

    protected:
        Output m_output;
    };
    using OutputLogFilter = std::shared_ptr<OutputLogFilter_t>;


    class FileLogFilter_t
        : public OutputLogFilter_t
    {
    public:
        FileLogFilter_t( FilePath dir = ".", String name = "", String ext = "log.txt", LogLevel level = LogLevel::Info, bool rotateFlag = true )
            : OutputLogFilter_t( nullptr, level ), m_dir( std::move( dir ) ), m_name( std::move( name ) ), m_ext( std::move( ext ) ), m_closeTime( 0 ), m_rotateFlag(rotateFlag) { open( DateTime::now() ); }
        ~FileLogFilter_t()
            { close(); }

        void flush() override
            { Lock lock( m_mutex ); if ( m_output ) { m_output.flush( ); } }
        void close( )
            { Lock lock( m_mutex ); m_output.close( ); m_filename.clear( ); }
        FilePath filename( )
            { Lock lock( m_mutex ); return m_filename; }
        void log( DateTime time, LogLevel level, const String & category, const String & message ) override;

    private:
        void open( DateTime time );

    private:
        Mutex m_mutex;
        FilePath m_dir;
        std::string m_name;
		std::string m_ext;
        FilePath m_filename;
        DateTime m_closeTime;
        bool m_rotateFlag;
    };
    using FileLogFilter = std::shared_ptr<FileLogFilter_t>;


    class DebugLogFilter_t
        : public AbstractLogFilter
    {
    public:
        DebugLogFilter_t( LogLevel level = LogLevel::Debug )
            : AbstractLogFilter( level ) { }

        void log( DateTime time, LogLevel level, const String & category, const String & message ) override;
    };
    using DebugLogFilter = std::shared_ptr<DebugLogFilter_t>;

}

