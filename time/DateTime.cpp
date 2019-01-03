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
		int diff = (int)( t - mktime( &gmt ) );

		/*
		if ( local.tm_isdst > 0 )
			{ diff += 1 * 60 * 60; }
		*/

		return diff;
	}

	Duration DateTime::localTimeDelta( )
	{
		static time_t timestamp = std::time( nullptr );
		static int diff = localTimeDiffSeconds( timestamp );

		time_t now = std::time( nullptr );
		if ( timestamp + 1 * 60 * 60 < now )
		{ 
			timestamp = now;
			diff = localTimeDiffSeconds( timestamp );
		}
		return cpp::Duration::ofSeconds( diff );
	}


	Duration steadyEpoch( )
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


	DateTime::DateTime( const Time & time )
		: m_sinceEpoch( steadyEpoch( ) + time.sinceEpoch( ) )
	{

	}


	DateTime::DateTime( const Date & date )
		: m_sinceEpoch( )
	{
		m_sinceEpoch = Duration::ofSeconds( mktime( ( std::tm * )&date.data( ) ) );

		//	adjust the time to UTC by removing the local time difference
		if ( date.isLocalTime( ) )
		{
			m_sinceEpoch -= localTimeDelta( );
		}
	}


	DateTime & DateTime::operator=( const Date & copy )
	{
		m_sinceEpoch = Duration::ofSeconds( mktime( ( std::tm * )&copy.data( ) ) );

		//	adjust the time to UTC by removing the local time difference
		if ( copy.isLocalTime( ) )
			{ m_sinceEpoch -= localTimeDelta( ); }

		return *this;
	}


	DateTime & DateTime::operator=( const Time & copy )
	{
		m_sinceEpoch = steadyEpoch( ) + copy.sinceEpoch( );
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

	Date DateTime::toDate( bool isLocalTime ) const
	{
		time_t t = to_time_t( );

		std::tm date;
		isLocalTime
			? localtime_s( &date, &t )
			: gmtime_s( &date, &t );

		return Date{ date, isLocalTime };
	}


	std::string DateTime::toString( bool isLocalTime ) const
	{
		std::string result = toString( "%Y-%m-%d %H:%M:%S", isLocalTime );
		result += "." + Integer::toDecimal( m_sinceEpoch.millis( ), 3, true );
		return result;
	}


	std::string DateTime::toString( const char * format, bool isLocalTime ) const
	{
		std::string result( 64, '\0' );

		Date date = toDate( isLocalTime );
		result.resize( strftime( (char *)result.c_str( ), result.length( ), format, &date.data( ) ) );

		return result;
	}


}