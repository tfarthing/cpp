#ifndef TEST

#include <cpp/chrono/Date.h>

namespace cpp
{
    cpp::Duration Date::utcDelta( )
    {
        time_t now = std::time( nullptr );
        std::tm local = *std::localtime( &now );
        std::tm gmt = *std::gmtime( &now );
        int diff = (int)( now - mktime( &gmt ) );

        if ( local.tm_isdst > 0 )
            { diff += 1 * 60 * 60; }

        return cpp::Duration::ofSeconds( diff );
    }
}

#else

#include <cpp/meta/Unittest.h>
#include <cpp/chrono/Date.h>

SUITE( Date )
{
    using namespace cpp;

    TEST( utcDelta )
    {
        DateTime now{};
        DateTime utc = now.toUTC( );
        DateTime local = utc.fromUTC( );

        Date utcDate = Date::fromTime( utc );
        Date localDate = Date::fromTime( local );

        CHECK( local == now );
    }

    /*
    TEST( localTimezone )
    {
        auto timezone = localTimezone();

        auto epochTime = DateTime::epoch( );
        auto gmtDate = Date::fromTime( epochTime, Timezone::GMT );
        auto localDate = Date::fromTime( epochTime, cpp::localTimezone( ) );

        CHECK( gmtDate.toTime( Timezone::GMT ).toEpochTime( ) == 0 );
        CHECK( localDate.toTime( cpp::localTimezone( ) ).toEpochTime( ) == cpp::localTimezone( ).toSeconds( ) );
    }

    TEST( fromTime )
    {
        // I was in California (PST) on Jan 1, 2001 at 01:01:01
        auto myWeirdDate = Date::create( 2001, 1, 1, 1, 1, 1 );
        CHECK( myWeirdDate.isDST( ) == false );
        // I had a datetime reference at this time
        auto myWeirdTime = myWeirdDate.toTime( Timezone::PST );
        //  The date was different at the same time in Greenwich
        auto gmtWeirdDate = Date::fromTime( myWeirdTime, Timezone::GMT );
        //  But the time from that date in Greenwich is no different from my time in California
        auto gmtWeirdTime = gmtWeirdDate.toTime( Timezone::GMT );

        CHECK( myWeirdTime == gmtWeirdTime );

        String msg = cpp::String::format( "In California at % it was % in Greenwich.", myWeirdDate.toString( ), gmtWeirdDate.toString( ) );
    }
    */

}

#endif
