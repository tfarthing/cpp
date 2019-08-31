#ifndef TEST

#include <ctime>
#include "../../cpp/time/Date.h"
#include "../../cpp/data/Integer.h"



namespace cpp
{

    std::string Date::toString( ) const
    {
        std::string result = toString( "%Y-%m-%d %H:%M:%S" );
        result += "." + Integer::toDecimal( m_micros / 1000, 3, true );
        return result;
    }


    std::string Date::toString( const char * format ) const
    {
        std::string result( 64, '\0' );

        result.resize( strftime( (char *)result.c_str( ), result.length( ), format, &data( ) ) );

        return result;
    }

}

#else

#include "../../cpp/meta/Test.h"
#include "../../cpp/time/Date.h"
#include "../../cpp/time/DateTime.h"


TEST_CASE( "Date" )
{
    using namespace cpp;

    SECTION( "utcDelta" )
    {
        DateTime now;

        Date localDate = now.toDate( );
        Date utcDate = now.toUtcDate( );

        DateTime localTime = localDate;
        DateTime utcTime = utcDate;

        CHECK( localTime == now );
        CHECK( utcTime == now );


        DateTime utc = DateTime::ofUtcDate( 1970, 1, 1 );
        CHECK( utc.sinceEpoch() == Duration::Zero );

        localDate = utc.toDate( );
        utcDate = utc.toUtcDate( );

        localTime = localDate;
        utcTime = utcDate;

        CHECK( localTime == utc );
        CHECK( utcTime == utc );
    }

}

#endif
