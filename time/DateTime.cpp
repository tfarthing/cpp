#include <cpp/chrono/DateTime.h>
#include <cpp/chrono/Date.h>

namespace cpp
{

    DateTime DateTime::fromUTC( )
    {
        return DateTime{ m_sinceEpoch + Date::utcDelta( ) };
    }

    DateTime DateTime::toUTC( )
    {
        return DateTime{ m_sinceEpoch - Date::utcDelta( ) };
    }

    DateTime DateTime::atStartOfHour( DateTime time )
    { 
        Date today;
        return Date::create( today.year( ), (int)today.month( ), today.day( ), today.hour( ), 0, 0 ).toTime( );
    }
    DateTime DateTime::atStartOfDay( DateTime time )
    { 
        Date today;
        return Date::create( today.year( ), (int)today.month( ), today.day( ), 0, 0, 0 ).toTime( );
    }

}