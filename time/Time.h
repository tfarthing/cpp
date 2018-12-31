#pragma once

#include <chrono>

#include "Duration.h"
#include "../process/Exception.h"



namespace cpp
{

	class DateTime;
    class Time
    {
    public:
        typedef std::chrono::steady_clock clock_t;

		static Time				now( );
		static Time				inFuture( Duration duration );
		static Time				inPast( Duration duration );

								Time( );
								Time( const Time & copy );
								Time( const DateTime & dateTime );
								Time( const clock_t::time_point & timepoint );
								Time( Duration sinceEpoch );

		Time &					operator=( const Time & copy );
		Time &					operator=( const DateTime & datetime );

								operator Time::clock_t::time_point( ) const;

		Duration				sinceEpoch( ) const;

		std::string				toString( const char * format ) const;
		std::string				toString( ) const;

		static int				compare( const Time & lhs, const Time & rhs );

    private:
        Duration				m_sinceEpoch;
    };


	inline Time Time::now( )
	{
		return Time{ };
	}

	inline Time::Time( )
		: m_sinceEpoch( clock_t::now( ).time_since_epoch( ) ) 
	{
	}


	inline Time::Time( const Time & copy )
		: m_sinceEpoch( copy.m_sinceEpoch )
	{
	}


	inline Time::Time( const clock_t::time_point & timepoint )
		: m_sinceEpoch( timepoint.time_since_epoch( ) ) 
	{ 
	}


	inline Time::Time( Duration sinceEpoch )
		: m_sinceEpoch( sinceEpoch )
	{
	}


	inline Time & Time::operator=( const Time & copy )
	{
		m_sinceEpoch = copy.m_sinceEpoch;
		return *this;
	}

	inline Duration Time::sinceEpoch( ) const
	{
		return m_sinceEpoch;
	}


	inline int Time::compare( const Time & lhs, const Time & rhs )
	{
		return Duration::compare( lhs.sinceEpoch( ), rhs.sinceEpoch( ) );
	}

}


inline bool operator==( const cpp::Time & lhs, const cpp::Time & rhs )
    { return cpp::Time::compare( lhs, rhs ) == 0; }
inline bool operator!=( const cpp::Time & lhs, const cpp::Time & rhs )
    { return cpp::Time::compare( lhs, rhs ) != 0; }
inline bool operator<( const cpp::Time & lhs, const cpp::Time & rhs )
    { return cpp::Time::compare( lhs, rhs ) < 0; }
inline bool operator<=( const cpp::Time & lhs, const cpp::Time & rhs )
    { return cpp::Time::compare( lhs, rhs ) <= 0; }
inline bool operator>( const cpp::Time & lhs, const cpp::Time & rhs )
    { return cpp::Time::compare( lhs, rhs ) > 0; }
inline bool operator>=( const cpp::Time & lhs, const cpp::Time & rhs )
    { return cpp::Time::compare( lhs, rhs ) >= 0; }


inline cpp::Time operator+( cpp::Time time, cpp::Duration duration )
{ 
	cpp::check<cpp::DurationException>( !duration.isInfinite( ),
		"Time::operator+() : Cannot relate Time and infinite Duration" );
	return cpp::Time{ time.sinceEpoch( ) + duration }; 
}


inline cpp::Time operator-( cpp::Time time, cpp::Duration duration )
{ 
	cpp::check<cpp::DurationException>( !duration.isInfinite( ),
		"Time::operator-() : Cannot relate Time and infinite Duration" );
	return cpp::Time{ time.sinceEpoch( ) - duration };
}


inline cpp::Time & operator+=( cpp::Time & time, const cpp::Duration duration )
{
	time = time + duration; return time;
}


inline cpp::Time & operator-=( cpp::Time & time, const cpp::Duration duration )
{
	time = time - duration; return time;
}


inline cpp::Duration operator-( cpp::Time time1, cpp::Time time2 )
{ 
	return time1.sinceEpoch( ) - time2.sinceEpoch( ); 
}
