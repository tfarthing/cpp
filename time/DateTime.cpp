#include "Time.h"
#include "DateTime.h"
#include "Date.h"

namespace cpp
{

	int localTimeDiffSeconds( time_t t )
	{
		std::tm local, gmt;
		localtime_s( &local, &t );
		gmtime_s( &gmt, &t );

		return (int)( t - mktime( &gmt ) );
	}


    //  recalculates local time delta from GMT on an hourly basis
	Duration DateTime::localTimeDelta( )
	{
		static time_t timestamp = std::time( nullptr );
		static int diff = localTimeDiffSeconds( timestamp );

		time_t now = std::time( nullptr );
		if ( timestamp + 60 * 60 < now )
		{ 
			timestamp = (now / (60 * 60)) * (60 * 60); // round timestamp to previous hour
			diff = localTimeDiffSeconds( timestamp );
		}
		return cpp::Duration::ofSeconds( diff );
	}


	Duration steadyUtcEpoch( )
	{
		static Time timestamp = Time( );
		static Duration epoch = DateTime::now( ).sinceEpoch( ) - timestamp.sinceEpoch( );

		Time now;
		if ( now > timestamp + Duration::ofMinutes( 1 ) )
		{
			epoch = DateTime::now( ).sinceEpoch( ) - now.sinceEpoch( );
			timestamp = now;
		}

		return epoch;
	}


    DateTime DateTime::ofDate( int year, int month, int day, int hour, int min, int sec, int micros )
    {
        std::tm input{ };
        input.tm_year = year - 1900;
        input.tm_mon = month - 1;
        input.tm_mday = day;
        input.tm_hour = hour;
        input.tm_min = min;
        input.tm_sec = sec;
        input.tm_isdst = -1;

        return DateTime{ Date{ input, micros, true } };
    }


    DateTime DateTime::ofUtcDate( int year, int month, int day, int hour, int min, int sec, int micros )
    {
        std::tm input{ };
        input.tm_year = year - 1900;
        input.tm_mon = month - 1;
        input.tm_mday = day;
        input.tm_hour = hour;
        input.tm_min = min;
        input.tm_sec = sec;
        input.tm_isdst = -1;

        return DateTime{ Date{ input, micros, false } };
    }


	DateTime::DateTime( const Time & time )
		: m_sinceEpoch( steadyUtcEpoch( ) + time.sinceEpoch( ) )
	{

	}


	DateTime::DateTime( const Date & date )
		: m_sinceEpoch( )
	{
        m_sinceEpoch = Duration::ofSeconds( mktime( ( std::tm * ) & date.data( ) ) ) + Duration::ofMicros( date.micros( ) );

		//	adjust the time to UTC by removing the local time difference
		if ( !date.isLocalTime( ) )
		    { m_sinceEpoch += localTimeDelta( ); }
	}


	DateTime & DateTime::operator=( const Date & date )
	{
		m_sinceEpoch = Duration::ofSeconds( mktime( ( std::tm * )& date.data( ) ) ) + Duration::ofMicros( date.micros( ) );

		//	adjust the time to UTC by removing the local time difference
		if ( !date.isLocalTime( ) )
			{ m_sinceEpoch += localTimeDelta( ); }

		return *this;
	}


	DateTime & DateTime::operator=( const Time & copy )
	{
		m_sinceEpoch = steadyUtcEpoch( ) + copy.sinceEpoch( );
		return *this;
	}


	DateTime::clock_t::time_point DateTime::to_time_point( ) const
	{
		return clock_t::time_point{ m_sinceEpoch.to_duration( ) };
	}

	time_t DateTime::to_time_t( ) const
	{
		return clock_t::to_time_t( clock_t::time_point{ m_sinceEpoch.to_duration( ) } );
	}


	Date DateTime::toDate( ) const
	{
        std::tm date;
        time_t t = to_time_t( );

		localtime_s( &date, &t );

		return Date{ date, m_sinceEpoch.micros( ) % 1000000, true };
	}


    Date DateTime::toUtcDate( ) const
    {
        std::tm date;
        time_t t = to_time_t( );

        gmtime_s( &date, &t );

        return Date{ date, m_sinceEpoch.micros( ) % 1000000, false };
    }


	std::string DateTime::toString( ) const
	{
        return toDate( ).toString( );
	}


	std::string DateTime::toString( const char * format ) const
	{
        return toDate( ).toString( format );
    }


}