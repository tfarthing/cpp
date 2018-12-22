#pragma once

#include <chrono>

#include <cpp/data/Comparable.h>
#include <cpp/time/Duration.h>
#include <cpp/time/DateTime.h>
#include <cpp/process/Exception.h>


namespace cpp
{


    class Time
        : public Comparable<Time>
    {
    public:
        typedef std::chrono::steady_clock clock_t;

        static Time now( )
            { return Time{ }; }
        static Time epoch( )
            { return Time{ clock_t::time_point{ } }; }
        static Time inFuture( Duration duration );
        static Time inPast( Duration duration );
        static Time fromDateTime( DateTime datetime )
        { 
            int64_t millisFromNow = datetime.sinceEpoch( ).millis( ) - DateTime::now( ).sinceEpoch( ).millis( );
            int64_t millisFromEpoch = now( ).sinceEpoch( ).millis( );
            //check<DurationException>( millisFromEpoch + millisFromNow >= 0, "Invalid conversion of DateTime to Time (too far in the past)" );
            return Time{ clock_t::time_point{ std::chrono::milliseconds( millisFromEpoch + millisFromNow ) } };
        }


        Time( )
            : m_sinceEpoch( clock_t::now( ).time_since_epoch( ) ) { }
        Time( const clock_t::time_point & timepoint )
            : m_sinceEpoch( timepoint.time_since_epoch() ) { }

        Duration sinceEpoch( ) const
            { return m_sinceEpoch; }

        clock_t::time_point toTimePoint( ) const
            { return clock_t::time_point{ m_sinceEpoch.std() }; }
        DateTime toDateTime( ) const
        {  
            int64_t millisFromNow = sinceEpoch( ).millis( ) - now( ).sinceEpoch( ).millis( );
            int64_t millisFromEpoch = DateTime::now( ).sinceEpoch( ).millis( );
            return DateTime{ Duration::ofMillis( millisFromEpoch + millisFromNow ) };
        }

        static int compare( const Time & lhs, const Time & rhs )
            { return Duration::compare( lhs.m_sinceEpoch, rhs.m_sinceEpoch ); }

        static void checkDuration( Duration duration )
            { check<DurationException>( !duration.isInfinite(), "Cannot relate Time from infinite Duration" ); }

    private:
        Duration m_sinceEpoch;
    };

   
}


inline cpp::Time operator+( cpp::Time time, cpp::Duration duration )
    { cpp::Time::checkDuration( duration ); return cpp::Time{ time.toTimePoint( ) + duration.std() }; }
    
inline cpp::Time operator-( cpp::Time time, cpp::Duration duration )
    { cpp::Time::checkDuration( duration ); return cpp::Time{ time.toTimePoint( ) - duration.std() }; }
    
inline cpp::Duration operator-( cpp::Time time1, cpp::Time time2 )
    { return time1.sinceEpoch( ) - time2.sinceEpoch( ); }
