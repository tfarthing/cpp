#ifndef TEST

#include <cpp/chrono/Time.h>

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
