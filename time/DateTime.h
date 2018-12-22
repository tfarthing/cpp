#pragma once

#include <ctime>
#include <chrono>
#include <cpp/Exception.h>
#include <cpp/chrono/Duration.h>
#include <cpp/util/Comparable.h>

namespace cpp
{

    class DateTime
        : public Comparable<DateTime>
    {
    public:
        static DateTime now( )
            { return DateTime{ }; }
        static DateTime epoch( )
            { return DateTime{ 0 }; }

        typedef std::chrono::system_clock clock_t;

        DateTime( )
            : m_sinceEpoch( clock_t::now( ).time_since_epoch( ) ) { }
        explicit DateTime( const clock_t::time_point & timepoint )
            : m_sinceEpoch( timepoint.time_since_epoch() ) { }
        explicit DateTime( time_t epochTime )
            : m_sinceEpoch( Duration::ofSeconds( epochTime ) ) { }
        explicit DateTime( Duration epochTime )
            : m_sinceEpoch( epochTime ) { checkDuration( m_sinceEpoch ); }

        Duration sinceEpoch( ) const
            { return m_sinceEpoch; }

        clock_t::time_point toTimePoint( ) const
            { return clock_t::time_point{ m_sinceEpoch.std() }; }
        std::time_t toEpochTime( ) const
            { return m_sinceEpoch.seconds( ); }

        static int compare( const DateTime & lhs, const DateTime & rhs )
            { return Duration::compare( lhs.m_sinceEpoch, rhs.m_sinceEpoch ); }

        DateTime fromUTC( );
        DateTime toUTC( );

        static DateTime atStartOfHour( DateTime time );
        static DateTime atStartOfDay( DateTime time );

        static void checkDuration( Duration duration )
            { check<DurationException>( !duration.isInfinite(), "Cannot relate DateTime from infinite Duration" ); }

    private:
        Duration m_sinceEpoch;
    };

}

inline cpp::DateTime operator+( const cpp::DateTime time, const cpp::Duration duration )
    { cpp::DateTime::checkDuration( duration ); return cpp::DateTime{ time.sinceEpoch( ) + duration }; }
inline cpp::DateTime & operator+=( cpp::DateTime & time, const cpp::Duration duration )
    { time = time + duration; return time; }
    
inline cpp::DateTime operator-( const cpp::DateTime time, const cpp::Duration duration )
    { cpp::DateTime::checkDuration( duration ); return cpp::DateTime{ time.sinceEpoch( ) - duration }; }
inline cpp::DateTime & operator-=( cpp::DateTime & time, const cpp::Duration duration )
    { time = time - duration; return time; }
    
inline cpp::Duration operator-( const cpp::DateTime time1, const cpp::DateTime time2 )
    { return time1.sinceEpoch( ) - time2.sinceEpoch( ); }
 
