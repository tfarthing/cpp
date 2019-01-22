#pragma once

/*

	DateTime is an abstraction of system_clock::time_point, and represents the system clock in UTC.  
	(1) The duration since epoch is always in UTC, not adjusted for local time.
	(2) Date conversions to DateTime are from local time by default.
	(3) Date conversions from DateTime are to local time by default.

*/

#include <ctime>
#include <chrono>
#include "Duration.h"

namespace cpp
{

	class Time;
	class Date;

    class DateTime
    {
    public:
		static DateTime						now( );
		static DateTime						epoch( );
		static DateTime						ofDate( int year, int month, int day, int hour = 0, int min = 0, int sec = 0, int micros = 0 );

        typedef std::chrono::system_clock	clock_t;

											DateTime( );
											DateTime( const clock_t::time_point & timepoint );
											explicit DateTime( time_t epochTimeSeconds );
											DateTime( Duration epochTimeUTC );
											DateTime( const Time & time );
											DateTime( const Date & date );

		DateTime &							operator=( const DateTime & copy );
		DateTime &							operator=( const Time & copy );
		DateTime &							operator=( const Date & copy );

		Duration							sinceEpoch( ) const;

		DateTime::clock_t::time_point		to_time_point( ) const;
		time_t								to_time_t( ) const;
		Date								toDate( bool localTime = true ) const;
		std::string							toString( bool localTime = true ) const;
		std::string							toString( const char * format, bool localTime = true ) const;

		static int							compare( const DateTime & lhs, const DateTime & rhs );

        static DateTime						trimAtHour( DateTime time );
        static DateTime						trimAtDay( DateTime time );

		static Duration						localTimeDelta( );


    private:
        Duration m_sinceEpoch;
    };


	inline DateTime DateTime::now( )
	{
		return DateTime{ };
	}


	inline DateTime DateTime::epoch( )
	{
		return DateTime{ 0 };
	}


	inline DateTime DateTime::ofDate( int year, int month, int day, int hour, int min, int sec, int micros )
	{
		std::tm input{ };
		input.tm_year = year - 1900;
		input.tm_mon = month - 1;
		input.tm_mday = day;
		input.tm_hour = hour;
		input.tm_min = min;
		input.tm_sec = sec;
		input.tm_isdst = -1;
		time_t epochSeconds = mktime( &input );
		//	adjust the time to UTC by removing the local time difference
		return DateTime{ Duration::ofSeconds( epochSeconds ) + Duration::ofMicros( micros ) - localTimeDelta( ) };
	}


	inline DateTime::DateTime( )
		: m_sinceEpoch( clock_t::now( ).time_since_epoch( ) ) 
	{ 
	}


	inline DateTime::DateTime( const clock_t::time_point & timepoint )
		: m_sinceEpoch( timepoint.time_since_epoch( ) ) 
	{ 
	}


	inline DateTime::DateTime( time_t epochTime )
		: m_sinceEpoch( clock_t::from_time_t( epochTime ).time_since_epoch( ) ) 
	{ 
	}


	inline DateTime::DateTime( Duration epochTime )
		: m_sinceEpoch( epochTime ) 
	{
	}


	inline DateTime & DateTime::operator=( const DateTime & copy )
	{
		m_sinceEpoch = copy.m_sinceEpoch;
		return *this;
	}

	inline Duration DateTime::sinceEpoch( ) const
	{
		return m_sinceEpoch;
	}

	inline DateTime DateTime::trimAtHour( DateTime time )
	{
		clock_t::time_point t = time.to_time_point();
		return std::chrono::floor<std::chrono::hours>( t );
	}

	inline DateTime DateTime::trimAtDay( DateTime time )
	{
		clock_t::time_point t = time.to_time_point( );
		return std::chrono::floor<std::chrono::duration<int64_t, std::ratio<86400>>>( t );
	}

	inline int DateTime::compare( const DateTime & lhs, const DateTime & rhs )
		{ return Duration::compare( lhs.sinceEpoch( ), rhs.sinceEpoch( ) ); }

}


inline bool operator==( const cpp::DateTime & lhs, const cpp::DateTime & rhs )
    { return cpp::DateTime::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::DateTime & lhs, const cpp::DateTime & rhs )
    { return cpp::DateTime::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::DateTime & lhs, const cpp::DateTime & rhs )
    { return cpp::DateTime::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::DateTime & lhs, const cpp::DateTime & rhs )
    { return cpp::DateTime::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::DateTime & lhs, const cpp::DateTime & rhs )
    { return cpp::DateTime::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::DateTime & lhs, const cpp::DateTime & rhs )
    { return cpp::DateTime::compare( lhs, rhs ) >= 0; }


inline cpp::DateTime operator+( cpp::DateTime time, cpp::Duration duration )
{
	cpp::check<cpp::DurationException>( !duration.isInfinite( ),
		"DateTime::operator+() : Cannot relate DateTime and infinite Duration" );
	return cpp::DateTime{ time.sinceEpoch( ) + duration };
}


inline cpp::DateTime operator-( cpp::DateTime time, cpp::Duration duration )
{
	cpp::check<cpp::DurationException>( !duration.isInfinite( ),
		"DateTime::operator-() : Cannot relate DateTime and infinite Duration" );
	return cpp::DateTime{ time.sinceEpoch( ) - duration };
}


inline cpp::DateTime & operator+=( cpp::DateTime & time, const cpp::Duration duration )
    { time = time + duration; return time; }


inline cpp::DateTime & operator-=( cpp::DateTime & time, const cpp::Duration duration )
    { time = time - duration; return time; }


inline cpp::Duration operator-( const cpp::DateTime time1, const cpp::DateTime time2 )
    { return time1.sinceEpoch( ) - time2.sinceEpoch( ); }
 
