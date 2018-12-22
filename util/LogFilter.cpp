#include <cpp/Integer.h>
#include <cpp/chrono/Date.h>
#include <cpp/util/LogFilter.h>
#include <cpp/util/Utf16.h>

namespace cpp
{

    String AbstractLogFilter::format( DateTime time, LogLevel level, const Memory & category, const Memory & message )
    {
        if (!m_headerFlag)
            { return message; }

        if ( time.sinceEpoch().millis() != m_lastTime.sinceEpoch().millis() || m_lastTimeString.empty( ) )
        {
            m_lastTime = time;
            auto millis = time.sinceEpoch( ).millis( ) % 1000;
            m_lastTimeString = cpp::String::format( "%.%", 
                Date::fromTime( time ).toString( ), 
                cpp::Integer::toDecimal( millis, 3, 0, true ) );
        }

        String temp;
        if ( !category.isEmpty() )
        {
            uint32_t len = ( ( adjustCategoryAvgSum( (uint32_t)category.length() ) / 8) + 1) * 8;
            temp = "[";
            temp += category;
            if ( temp.length() < len + 1 )
                { temp.resize( len + 1, ' ' ); }
            temp += "] ";
        }

        String lvl = toString(level); lvl.resize(8, ' ');
        return String::format( "% % %%\r\n",
            m_lastTimeString,
            lvl,
            temp,
            message);
    }

    Memory::Array AbstractLogFilter::split( const Memory & message )
    {
        auto lines = message.split( "\n", "\r", false );
        if (lines.size() > 0 && lines.back().isEmpty())
            { lines.pop_back(); }
        return lines;
    }

    void OutputLogFilter_t::log( DateTime time, LogLevel level, const String & category, const String & message )
    { 
        if (level > m_level) 
            { return; } 

        auto lines = split( message );
        for ( auto & line : lines )
        {
            m_output.print( format( time, level, category, line ) );
        }
    }

    void FileLogFilter_t::open( DateTime time )
    {
        if ( m_rotateFlag )
        {
            m_closeTime = DateTime::atStartOfDay( time ) + Duration::ofDays( 1 );
            String name = String::format( "%%%",
                Date::fromTime( time ).toString( "%y-%m-%d" ),
                m_name.isEmpty( ) ? "" : "-" + m_name,
                m_ext.isEmpty( ) ? "" : "." + m_ext );
            m_filename = FilePath{ m_dir, name };
        }
        else
        {
            m_filename = FilePath{ m_dir, m_name + "." + m_ext };
        }
        m_output = File::append( m_filename, cpp::File::Share::AllowWrite );
    }

    void FileLogFilter_t::log( DateTime time, LogLevel level, const String & category, const String & message )
    {
        Lock lock(m_mutex);
        if ( level > m_level ) 
            { return; } 
        if ( m_rotateFlag && time >= m_closeTime && m_output.isOpen() )
            { m_output.close(); m_filename.clear(); }
        if ( !m_output )
            { open( time ); }
        OutputLogFilter_t::log( time, level, category, message );
    }

    void DebugLogFilter_t::log( DateTime time, LogLevel level, const String & category, const String & message )
    {
#ifdef _DEBUG
        if (level > m_level) 
            { return; } 

        auto lines = split( message );
        for (auto & line : lines)
        {
            OutputDebugString( toUtf16( format( time, level, category, line ) ).data() ); 
        }
#endif
    }

}