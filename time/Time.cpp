#ifndef TEST

#include "Time.h"
#include "DateTime.h"

namespace cpp
{

	Time Time::inFuture( Duration duration )
	{
		return now( ) + duration;
	}


	Time Time::inPast( Duration duration )
	{
		return now( ) - duration;
	}


	Time::Time( const DateTime & datetime )
		: m_sinceEpoch( clock_t::now( ).time_since_epoch( ) + ( datetime - DateTime::now( ) ) )
	{
	}


	Time & Time::operator=( const DateTime & datetime )
	{
		m_sinceEpoch = clock_t::now( ).time_since_epoch( ) + ( datetime - DateTime::now( ) );
		return *this;
	}


	Time::operator Time::clock_t::time_point( ) const
	{
		return clock_t::time_point{ ( std::chrono::microseconds )m_sinceEpoch };
	}


	std::string Time::toString( const char * format ) const
	{
		return DateTime{ *this }.toString( format );
	}


	std::string Time::toString( ) const
	{
		return DateTime{ *this }.toString( );
	}

}

#else

#include <cpp/meta/Unittest.h>
#include <cpp/chrono/Time.h>

SUITE( Time )
{
    using namespace cpp;

    TEST( ctor )
    {
        Time a = Time::epoch();
        Time b = a + Duration::ofSeconds( 1000000 );

        CHECK( a.sinceEpoch( ).seconds( ) == 0 );
        CHECK( b.sinceEpoch( ).seconds( ) == 1000000 );
    }

}

#endif
